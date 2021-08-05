#pragma once
#include "modio/detail/ops/auth/AuthenticateUserExternal.h"
#include "modio/detail/AsioWrapper.h"
namespace Modio
{
	namespace Detail
	{
		inline void AuthenticateUserByDiscordAsync(Modio::AuthenticationParams User,
										std::function<void(Modio::ErrorCode)> Callback)
		{
			Modio::Detail::HttpRequestParams Params =
				Modio::Detail::AuthenticateViaDiscordRequest
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::DiscordToken, User.AuthToken)
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::EmailAddress, User.UserEmail)
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::TermsAgreed,
										User.bUserHasAcceptedTerms ? "true" : "false");
			return asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
				Modio::Detail::AuthenticateUserExternal(Params), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio