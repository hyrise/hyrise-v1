// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_INSERTONLY_H_
#define SRC_LIB_ACCESS_INSERTONLY_H_

#include "helper/types.h"

namespace hyrise {
namespace insertonly {
/// InsertOnly implementation
///
/// Note: the current implementation only works for single-threaded workloads
///
/// An InsertOnly table is constructed by attaching 2 extra columns
/// to the original table, named "$FROM" and "$TO". The implementation closely
/// follows the idea of `interval representation` insert only.
///
/// A key difference to most interval representations in our implementation
/// lies in currently using a magic value (0) to represent visibility in the
/// $TO column. This leads to an extra comparison per row when creating the
/// valid view of a column but allows us to use the sorted dictionary backing
/// columns instead of resorting to an unsorted dictionary.

/// Create an insert only table from a csv file
/// @returns table that fulfills insert-only criteria
/// @param[in] filename path to file
/// @param[in] tid transaction id to specify when the table was loaded
storage::atable_ptr_t construct(std::string filename,
                                const tx::transaction_id_t& tid);

/// Check whether a table is actually an insert only table
/// @param[in] table table to check
/// @returns pointer to table when table is indeed insert only, otherwise nullptr
storage::c_simplestore_ptr_t isInsertOnly(const storage::c_atable_ptr_t& table);

/// Assures an insertonly table is used, throws exception
/// @param[in] table to check
/// @return pointer to table
/// @throws std::runtime error when a non-insert-only table is passed
storage::simplestore_ptr_t assureInsertOnly(const storage::c_atable_ptr_t& table);

/// Insert rows into a table that fulfills criteria of an insert-only table
/// @param[in,out] store table is extended with data defined in rows
/// @param[in] rows table that is used to extend store - has to have all data columns of store
/// @param[in] tid transaction id at which rows are inserted
void insertRows(const storage::simplestore_ptr_t& store,
                const storage::c_atable_ptr_t& rows,
                const tx::transaction_id_t& tid);

/// Delete rows from a table that fulfills criteria of an insert-only table
/// @param[in,out] store table is extended with data defined in rows
/// @param[in] positions list of positions that will be made invisible from tid on
/// @param[in] tid transaction id at which rows are made invisible
void deleteRows(const storage::simplestore_ptr_t& store,
                const storage::pos_list_t& positions,
                const tx::transaction_id_t& tid);

/// This version allows for separately accessing main and delta
void deleteRows(const storage::simplestore_ptr_t& store,
                const storage::pos_list_t& positions_main,
                const storage::pos_list_t& positions_delta,
                const tx::transaction_id_t& tid);

/// Update rows in a table that fulfills criteria of an insert-only table
/// @param[in,out] store table is extended with data defined in rows
/// @param[in] rows table that is used to extend store - has to have all data columns of store
/// @param[in] update_positions list of positions that will be made invisible from tid on
/// @param[in] tid transaction id at which rows are made invisible
void updateRows(const storage::simplestore_ptr_t& store,
                const storage::c_atable_ptr_t& rows,
                const storage::pos_list_t& update_positions,
                const tx::transaction_id_t& tid);

/// This version allows for separately updating main and delta
void updateRows(const storage::simplestore_ptr_t& store,
                const storage::c_atable_ptr_t& rows,
                const storage::pos_list_t& update_positions_main,
                const storage::pos_list_t& update_positions_delta,
                const tx::transaction_id_t& tid);

/// Return valid view on insert-only table
/// @param[in] store table to be filtered for valid entries
/// @param[in] tid transaction id to be used as world state
/// @returns A table only containing visible entries
storage::atable_ptr_t filterValid(const storage::simplestore_ptr_t& store,
                                  const tx::transaction_id_t& tid);

/// Return valid positions of delta part of store (as pointer calculator)
/// @param[in] store table to extract delta from
/// @param[in] tid transaction id to be used as world state
/// @returns a pointer calculator on delta (that cannot be used)
storage::atable_ptr_t validPositionsDelta(const storage::simplestore_ptr_t& store,
                                          const tx::transaction_id_t& tid);

/// Return valid positions of main part of store (as pointer calculator)
/// @param[in] store table to extract delta from
/// @param[in] tid transaction id to be used as world state
/// @returns a pointer calculator on delta (that cannot be used)
storage::atable_ptr_t validPositionsMain(const storage::simplestore_ptr_t& store,
                                         const tx::transaction_id_t& tid);

/// Merges a store to a specific transaction id
storage::atable_ptr_t merge(const storage::simplestore_ptr_t& store,
                            const tx::transaction_id_t& tid);


}}


#endif
