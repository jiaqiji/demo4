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
// Created by WangYunlai on 2022/6/9.
//

#pragma once

#include "sql/operator/physical_operator.h"
#include "sql/parser/value.h"
class Trx;
class UpdateStmt;

/**
 * @brief 物理算子，删除
 * @ingroup PhysicalOperator
 */
class UpdatePhysicalOperator : public PhysicalOperator
{
public:
  UpdatePhysicalOperator(Table *table, std::string keyID, Value *v, int count)
      : table_(table), values_(v), value_amount_(count)
  {
    attribute_KeyID = keyID;
  }
  virtual ~UpdatePhysicalOperator() = default;
  PhysicalOperatorType type() const override { return PhysicalOperatorType::UPDATE; }
  RC                   open(Trx *trx) override;
  RC                   next() override;
  RC                   close() override;
  Tuple               *current_tuple() override { return nullptr; }
  std::string          updateID() const { return attribute_KeyID; }

private:
  std::string attribute_KeyID;  ///< 更新的字段，仅支持一个字段

  Table *table_        = nullptr;
  Trx   *trx_          = nullptr;
  Value *values_       = nullptr;
  int    value_amount_ = 0;
};
