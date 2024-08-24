#ifndef _IO_H
#define _IO_H

#include "grpc/procmon.grpc.pb.h"

/**
 * @description: Open the SQLite3 database
 * @param {*}
 * @return {int} - return SQLITE_OK on success, otherwise an error code
 */
int open_db();

/**
 * @description: Retrieve system information from the database and populate the gRPC response
 * @param {procmon::StatusResponse*} response - pointer to the gRPC response object
 * @return {int} - return SQLITE_OK on success, otherwise an error code
 */
int get_system_info(procmon::StatusResponse* response);

/**
 * @description: Retrieve nft traffic information from the database and populate the gRPC response
 * @param {procmon::NftTrafficResponse*} response - pointer to the gRPC response object
 * @return {int} - return SQLITE_OK on success, otherwise an error code
 */
int get_nft_traffic(procmon::NftTrafficResponse* response);

/**
 * @description: Delete the SQLite3 database file
 * @param {*}
 * @return {*}
 */
void delete_db();

#endif