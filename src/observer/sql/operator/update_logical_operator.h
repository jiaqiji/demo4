/* Copyright (c) OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by WangYunlai on 2022/12/26.
//

#pragma once
#include "sql/operator/logical_operator.h"
#include "sql/parser/value.h"
/**
 * @brief
 * @ingroup LogicalOperator
 */
class UpdateLogicalOperator : public LogicalOperator
{
public:
  UpdateLogicalOperator(Table *table, std::string keyID, Value *v, int count)
      : table_(table), values_(v), value_amount_(count)
  {
    attribute_KeyID = keyID;
  }
  virtual ~UpdateLogicalOperator() = default;
  LogicalOperatorType type() const override { return LogicalOperatorType::UPDATE; }
  Table              *table() const { return table_; }
  Value              *values() const { return values_; }
  int                 value_amount() const { return value_amount_; }
  std::string         updateID() const { return attribute_KeyID; }

private:
  std::string attribute_KeyID;  ///< 更新的字段，仅支持一个字段

  Table *table_        = nullptr;
  Value *values_       = nullptr;
  int    value_amount_ = 0;
};
