#pragma once
#include "modio/detail/CoreOps.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/detail/AsioWrapper.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class RequestEmailAuthCodeOp
		{
		public:
			RequestEmailAuthCodeOp(std::string EmailAddress) : EmailAddress(EmailAddress) {};
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						ResponseBuffer, Modio::Detail::RequestEmailSecurityCodeRequest.AppendPayloadValue("email", EmailAddress), CachedResponse::Disallow,
						std::move(Self));
					
					Self.complete(ec);
					return;
				}
			}

		private:
			asio::coroutine CoroutineState;
			Modio::Detail::DynamicBuffer ResponseBuffer;
			std::string EmailAddress;
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>