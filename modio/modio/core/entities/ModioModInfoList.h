#pragma once
#include "ModioGeneratedVariables.h"
#include "modio/detail/ModioDefines.h"

#include "ModioModInfo.h"
#include "ModioPagedResult.h"
#include "modio/detail/ModioJsonHelpers.h"
#include <vector>

namespace Modio
{

	/// @brief Class representing a list of mods that may be a page from a larger set of results
	class ModInfoList : public PagedResult, public List<std::vector, Modio::ModInfo>
	{
	public:
		void Append(const ModInfoList& Other)
		{
			InternalList.insert(InternalList.end(), std::begin(Other.InternalList), std::end(Other.InternalList));
		}
		friend inline void from_json(const nlohmann::json& Json, Modio::ModInfoList& OutModInfoList);
	};

	void from_json(const nlohmann::json& Json, Modio::ModInfoList& OutModInfoList)
	{
		from_json(Json, static_cast<Modio::PagedResult&>(OutModInfoList));

		Detail::ParseSafe(Json, OutModInfoList.InternalList, "data");
	}
} // namespace Modio