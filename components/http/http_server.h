/**
 * @file http_server.h
 * @brief HTTP server API for light control and OTA
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#ifndef COMPONENTS_HTTP_HTTP_SERVER_H
#define COMPONENTS_HTTP_HTTP_SERVER_H

#include "esp_err.h"
#include "esp_http_server.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Initialize the HTTP server module.
 *
 * Performs basic initialization of the HTTP server module without starting
 * the actual server. This allows configuration to be set before the server
 * begins accepting connections.
 *
 * @return
 *      - ESP_OK on successful initialization
 *      - ESP_FAIL if server is already initialized
 *
 * @note This function only initializes the module state; use http_server_start()
 *       to begin accepting HTTP connections.
 */
esp_err_t http_server_init(void);

/**
 * @brief Start the HTTP server and begin accepting connections.
 *
 * Creates and starts the HTTP server with the configured parameters,
 * then registers all defined routes. The server will begin accepting
 * client connections on the specified port after successful startup.
 *
 * @return
 *      - ESP_OK on successful server start and route registration
 *      - ESP_FAIL if server is already running
 *      - ESP_ERR_* on HTTP server creation or route registration failure
 *
 * @note The server uses the configuration set via http_server_set_* functions.
 *       All routes are automatically registered upon successful startup.
 */
esp_err_t http_server_start(void);

/**
 * @brief Stop the HTTP server and close all connections.
 *
 * Gracefully shuts down the HTTP server, closing all active client
 * connections and freeing associated resources. The server will stop
 * accepting new connections immediately.
 *
 * @return
 *      - ESP_OK on successful server shutdown
 *      - ESP_FAIL if server was not running
 *      - ESP_ERR_* on shutdown failure
 *
 * @note After stopping, the server can be restarted with http_server_start().
 */
esp_err_t http_server_stop(void);

/**
 * @brief Check if the HTTP server is currently running.
 *
 * Returns the current operational status of the HTTP server, indicating
 * whether it is actively accepting and processing client connections.
 *
 * @return
 *      - true if the server is running and accepting connections
 *      - false if the server is stopped or not initialized
 */
bool http_server_is_running(void);

/* === Server configuration === */
/**
 * @brief Set the TCP port for the HTTP server.
 *
 * Configures the port number on which the HTTP server will listen for
 * incoming connections. This setting only takes effect when the server
 * is started; it cannot be changed while the server is running.
 *
 * @param[in] port  TCP port number (typically 80 for HTTP, 443 for HTTPS).
 *
 * @note Changes only apply to future server starts. Stop and restart
 *       the server to apply port changes.
 */
void http_server_set_port(uint16_t port);

/**
 * @brief Set the stack size for HTTP server tasks.
 *
 * Configures the amount of memory allocated for the HTTP server's task
 * stack. Larger stack sizes may be needed for complex request handlers
 * or when processing large amounts of data.
 *
 * @param[in] stack_size  Stack size in bytes for HTTP server tasks.
 *
 * @note Changes only apply to future server starts. The default stack
 *       size is usually sufficient for basic operations.
 */
void http_server_set_stack_size(size_t stack_size);

/**
 * @brief Set the maximum number of simultaneous connections.
 *
 * Configures the limit for concurrent client connections that the HTTP
 * server will accept. Additional connection attempts will be rejected
 * when this limit is reached.
 *
 * @param[in] max_connections  Maximum number of simultaneous connections.
 *
 * @note Lower values reduce memory usage but may limit concurrent access.
 *       Higher values increase resource usage but improve accessibility.
 */
void http_server_set_max_connections(int max_connections);

#endif // COMPONENTS_HTTP_HTTP_SERVER_H
