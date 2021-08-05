#pragma once

#include "modio/compression/ModioArchiveFile.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/file/ModioFileService.h"

#include <asio/yield.hpp>

class ExtractAllToFolderOp
{
	struct ExtractAllImpl
	{
		asio::coroutine CoroutineState;
		Modio::filesystem::path ArchivePath;
		Modio::filesystem::path RootOutputPath;
		Modio::Detail::ArchiveFile ArchiveView;
		Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo;

		ExtractAllImpl(asio::coroutine CoroutineState, Modio::filesystem::path ArchivePath,
					   Modio::filesystem::path RootOutputPath, Modio::Detail::ArchiveFile ArchiveView,
					   Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo)
			: CoroutineState(CoroutineState),
			  ArchivePath(ArchivePath),
			  RootOutputPath(RootOutputPath),
			  ArchiveView(std::move(ArchiveView)),
			  ProgressInfo(ProgressInfo) {};
	};
	Modio::StableStorage<ExtractAllImpl> Impl;
	std::vector<Modio::Detail::ArchiveFileImplementation::ArchiveEntry>::iterator CurrentEntryIterator;

public:
	ExtractAllToFolderOp(Modio::filesystem::path ArchivePath, Modio::filesystem::path RootOutputPath,
						 Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo)
	{
		Impl = std::make_shared<ExtractAllImpl>(
			ExtractAllImpl {{}, ArchivePath, RootOutputPath, Modio::Detail::ArchiveFile(ArchivePath), ProgressInfo});
	};

	template<typename CoroType>
	void operator()(CoroType& Self, Modio::ErrorCode ec = {})
	{
		reenter(Impl->CoroutineState)
		{
			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Compression,
										"Extracting archive {}", Impl->ArchivePath.u8string());
			// Parsing archive contents is an async operation because it requires reading/seeking around in the archive
			// file which is async
			yield Impl->ArchiveView.ParseArchiveContentsAsync(std::move(Self));

			if (ec)
			{
				Self.complete(ec, Modio::FileSize(0));
				return;
			}

			if (!Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().CheckSpaceAvailable(
					Impl->RootOutputPath, Impl->ArchiveView.GetTotalExtractedSize()))
			{
				Self.complete(Modio::make_error_code(Modio::FilesystemError::InsufficientSpace), Modio::FileSize(0));
				return;
			}

			// Update the total size of the archive
			if (Impl->ProgressInfo.has_value())
			{
				if (!Impl->ProgressInfo->expired())
				{
					Impl->ProgressInfo->lock()->TotalExtractedSizeOnDisk = Impl->ArchiveView.GetTotalExtractedSize();
				}
				else
				{
					Self.complete(Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled),
								  Modio::FileSize(0));
					return;
				}
			}

			// Now we know what entries are in the zip file, we need to loop through them and extract them to disk
			CurrentEntryIterator = Impl->ArchiveView.begin();

			while (CurrentEntryIterator != Impl->ArchiveView.end())
			{
				// If the current entry has no filename (ie it is just a directory), create that directory
				if (!CurrentEntryIterator->FilePath.has_filename())
				{
					Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().CreateFolder(
						Impl->RootOutputPath / CurrentEntryIterator->FilePath);
				}
				else
				{
					// Otherwise, begin an async operation to extract that entry to a file with the path based on the
					// root path
					yield Impl->ArchiveView.ExtractEntryAsync(*CurrentEntryIterator, Impl->RootOutputPath,
															  Impl->ProgressInfo, std::move(Self));
					if (ec)
					{
						Self.complete(ec, Modio::FileSize(0));
						return;
					}
				}
				CurrentEntryIterator++;
			}
			// Once all files are extracted, notify the caller
			Self.complete(Modio::ErrorCode {}, Impl->ArchiveView.GetTotalExtractedSize());
		}
	}
};

#include <asio/unyield.hpp>
