/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#pragma once

#include "common/rc.h"
#include "sql/stmt/stmt.h"
// #include "sql/parser/value.h"

class Table;
class Value;
class FilterStmt;
/**
 * @brief 更新语句
 * @ingroup Statement
 */
class UpdateStmt : public Stmt
{
public:
  UpdateStmt() = default;
  UpdateStmt(Table *table, Value *values, int value_amount);
  UpdateStmt(Table *table, FilterStmt *filter_stmt, std::string keyID, Value *values, int value_amount)
  {
    table_          = table;
    filter_stmt_    = filter_stmt;
    values_         = values;
    value_amount_   = value_amount;
    attribute_KeyID = keyID;
  }
  ~UpdateStmt() override;

public:
  static RC create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt);

public:
  Table      *table() const { return table_; }
  std::string updateID() const { return attribute_KeyID; }
  Value      *values() const { return values_; }
  int         value_amount() const { return value_amount_; }

  FilterStmt *filter_stmt() const { return filter_stmt_; }

  StmtType type() const override { return StmtType::UPDATE; }

private:
  std::string attribute_KeyID;  ///< 更新的字段，仅支持一个字段

  Table      *table_        = nullptr;
  Value      *values_       = nullptr;
  int         value_amount_ = 0;
  FilterStmt *filter_stmt_  = nullptr;
};
