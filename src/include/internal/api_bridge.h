/**
 * @file api_bridge.h
 * @author soon (kento.soon@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-09-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OCTANE_API_CLIENT_INTERNAL_API_BRIDGE_H_
#define OCTANE_API_CLIENT_INTERNAL_API_BRIDGE_H_

#include "include/api_result_types.h"
#include "include/error_response.h"
#include "include/internal/fetch.h"
#include "include/result.h"

namespace octane::internal {
  class ApiBridge {
    internal::Fetch fetch;

  public:
    /**
     * @brief Construct a new Api Bridge object
     *
     * @param[in] token
     * @param[in] origin http://localhost:3000
     * @param[in] baseUrl /api/v1
     */
    ApiBridge(std::string_view token,
              std::string_view origin,
              std::string_view baseUrl);
    /**
     * @brief Initialize
     *
     * @return Result<std::optional<std::string>, ErrorResponse>
     */
    Result<_, ErrorResponse> init();
    /**
     * @brief use get method for health api
     *
     * @return Result<HealthResult, ErrorResponse>
     */
    Result<HealthResult, ErrorResponse> healthGet();
    /**
     * @brief use post method for room api
     *
     * @param[in] name
     * @return Result<std::string, ErrorResponse>
     */
    Result<std::string, ErrorResponse> roomPost(std::string_view name);
    /**
     * @brief use get method for room/:id api
     *
     * @param[in] id
     * @return Result<RoomStatus, ErrorResponse>
     */
    Result<RoomStatus, ErrorResponse> roomIdGet(std::string_view id);
    /**
     * @brief use delete method for room/:id api
     *
     * @param[in] id
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> roomIdDelete(std::string_view id);
    /**
     * @brief use post method for room/:id api
     *
     * @param[in] id
     * @param[in] name
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> roomIdPost(std::string_view id,
                                        std::string_view name);
    /**
     * @brief use get method for room/:id/content api
     *
     * @param[in] id
     * @return Result<Content, ErrorResponse>
     */
    Result<Content, ErrorResponse> roomIdContentGet(std::string_view id);
    /**
     * @brief use delete method for room/:id/content api
     *
     * @param[in] id
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> roomIdContentDelete(std::string_view id);
    /**
     * @brief use put method for room/:id/content api
     *
     * @param[in] id
     * @param[in] content
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> roomIdContentPut(std::string_view id,
                                              const Content& content);
    /**
     * @brief use get method for room/:id/status api
     *
     * @param[in] id
     * @return Result<internal::ContentStatus, ErrorResponse>
     */
    Result<internal::ContentStatus, ErrorResponse> roomIdStatusGet(
      std::string_view id);
    /**
     * @brief use delete method for room/:id/status api
     *
     * @param[in] id
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> roomIdStatusDelete(std::string_view id);
    /**
     * @brief use put method for room/:id/status api
     *
     * @param[in] id
     * @param[in] contentStatus
     * @return Result<_, ErrorResponse>
     */
    Result<_, ErrorResponse> roomIdStatusPut(
      std::string_view id,
      const internal::ContentStatus& contentStatus);
    /**
     * @brief check if the given status code is 2xx
     * 
     * @param response 
     * @return std::optional<error_t<ErrorResponse>> 
     */
    std::optional<error_t<ErrorResponse>> checkStatusCode(
      const internal::FetchResponse& response);
  };
} // namespace octane::internal
#endif // OCTANE_API_CLIENT_INTERNAL_API_BRIDGE_H_