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
// Created by Wangyunlai on 2022/07/05.
//

#include "sql/expr/expression.h"
#include "expression.h"
#include "sql/expr/tuple.h"
#include <string.h>

using namespace std;

RC FieldExpr::get_value(const Tuple &tuple, Value &value) const {
    return tuple.find_cell(TupleCellSpec(table_name(), field_name()), value);
}

RC ValueExpr::get_value(const Tuple &tuple, Value &value) const {
    value = value_;
    return RC::SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////
CastExpr::CastExpr(unique_ptr<Expression> child, AttrType cast_type) :
        child_(std::move(child)), cast_type_(cast_type) {}

CastExpr::~CastExpr() {}

RC CastExpr::cast(const Value &value, Value &cast_value) const {
    RC rc = RC::SUCCESS;
    if (this->value_type() == value.attr_type()) {
        cast_value = value;
        return rc;
    }

    switch (cast_type_) {
    case BOOLEANS: {
        bool val = value.get_boolean();
        cast_value.set_boolean(val);
    } break;
    default: {
        rc = RC::INTERNAL;
        LOG_WARN("unsupported convert from type %d to %d", child_->value_type(), cast_type_);
    }
    }
    return rc;
}

RC CastExpr::get_value(const Tuple &tuple, Value &cell) const {
    RC rc = child_->get_value(tuple, cell);
    if (rc != RC::SUCCESS) {
        return rc;
    }

    return cast(cell, cell);
}

RC CastExpr::try_get_value(Value &value) const {
    RC rc = child_->try_get_value(value);
    if (rc != RC::SUCCESS) {
        return rc;
    }

    return cast(value, value);
}

////////////////////////////////////////////////////////////////////////////////

ComparisonExpr::ComparisonExpr(CompOp comp, unique_ptr<Expression> left, unique_ptr<Expression> right) :
        comp_(comp), left_(std::move(left)), right_(std::move(right)) {}

ComparisonExpr::~ComparisonExpr() {}

int _strat(char *chText, char *chPattern, int nbegpos /* = 0 */, int nlen /* = -1 */, bool bleft /* = false */)
{
    int nPatternLen = strlen(chPattern);
    int nTextLen = strlen(chText);
    if (nlen >= 0)
    {
        if (nbegpos + nlen < nTextLen)
            nTextLen = nbegpos + nlen;
    }
    if (nbegpos + nPatternLen > nTextLen || nPatternLen > 256)
        return -1;
    if (nPatternLen == 0)
        return nbegpos;
    else
    {
        int nGeneralLen = 0;
        short chNext[256] = { -1 };
        int nPattPos = 0, nNext = -1;
        if (!bleft)
        {
            // 生成模式回退值
            while (nPattPos < nPatternLen)
            {
                if (nNext == -1 || chPattern[nPattPos] == '_' || chPattern[nPattPos] == chPattern[nNext])
                {
                    nPattPos++;
                    nNext++;
                    chNext[nPattPos] = nNext;
                }
                else
                    nNext = chNext[nNext];
            }
        }
        int nTextPos = nbegpos;
        nPattPos = 0;
        // 进行模式匹配
        while (nPattPos < nPatternLen && nTextPos < nTextLen)
        {
            if (nPattPos == -1 || chPattern[nPattPos] == '_' || chPattern[nPattPos] == chText[nTextPos])
            {
                nPattPos++;
                nTextPos++;
            }
            else
            {
                // 求左对齐时，不允许回退（回退时肯定不是左对齐的）
                if (bleft)
                    return -1;
                else
                    nPattPos = chNext[nPattPos];
            }
        }
        // 判断模式串是否已经完全被匹配，否则返回-1
        if (nPattPos == nPatternLen)
            return nTextPos - nPattPos;
        else
            return -1;
    }
}

bool _strlike(char *chText, char *chPattern, int nbegpos /* = 0 */)
{
    bool bLeftMatch = true, bLast = false;
    int nTextLen = strlen(chText);
    // 作最基础的匹配，即存在模式串的情况下再作比较
    if (strlen(chPattern) == 0)
        if (strlen(chText) == 0)
            return true;
        else
            return false;
    do
    {
        char *chFirstPattern, *chSecondPattern;
        if (chPattern[0] == '%')
        {
            do
            {
                chPattern++;
            } while (chPattern[0] == '%');
            if (chPattern == NULL || strlen(chPattern) == 0)
                return true;
            bLeftMatch = false;
        }
        else
            bLeftMatch = true;
        // 初始化模式串
        chSecondPattern = strrchr(chPattern, (int)('%'));
        
        int nPatternLen;
        if (chSecondPattern == NULL)
        {
            bLast = true;
            nPatternLen = strlen(chPattern);
            if (!bLeftMatch)
            {
                // 若以%开头，并且没有剩余模式串时，只要考虑右对齐匹配的方式即可(实际上也是左对齐)
                if (nbegpos + nPatternLen <= nTextLen)
                {
                    nbegpos = nTextLen - nPatternLen;
                    bLeftMatch = true;
                }
                else
                    return false;
            }
            else if (nbegpos + nPatternLen != nTextLen)
                return false;
        }
        else
        {
            // 模式串不得长于原串
            nPatternLen = chSecondPattern - chPattern;
            if (nbegpos + nPatternLen > nTextLen)
                return false;
        }
        // 初始化模式串与修改剩余串
        chFirstPattern = new char[nPatternLen + 1];
        memcpy(chFirstPattern, chPattern, nPatternLen);
        chFirstPattern[nPatternLen] = 0;
        chPattern = chSecondPattern;
        int npos = _strat(chText, chFirstPattern, nbegpos, bLeftMatch ? nPatternLen : nTextLen - nbegpos, bLeftMatch);
        delete chFirstPattern;
        if (npos < 0)
        {
            return false;
        }
        else
        {
            // 定下一查找位置的起点
            if (bLeftMatch)
            {
                if (npos != nbegpos)
                    return false;
            }
            else
                nbegpos = npos;
            if (bLast)
            {
                if (nPatternLen + npos == nTextLen)
                    return true;
                else
                    return false;
            }
            else
                nbegpos += nPatternLen;
        }

    } while (true);
}

RC ComparisonExpr::compare_value(const Value &left, const Value &right, bool &result) const {
    RC rc = RC::SUCCESS;
    int cmp_result = left.compare(right);
    result = false;
    std::string sleft;
    std::string sright;
    bool    bLike = false;
    switch (comp_) {
    case EQUAL_TO: {
        result = (0 == cmp_result);
    } break;
    case LESS_EQUAL: {
        result = (cmp_result <= 0);
    } break;
    case NOT_EQUAL: {
        result = (cmp_result != 0);
    } break;
    case LESS_THAN: {
        result = (cmp_result < 0);
    } break;
    case GREAT_EQUAL: {
        result = (cmp_result >= 0);
    } break;
    case GREAT_THAN: {
        result = (cmp_result > 0);
    } break;
    case LIKE_OP: {
        sleft = left.get_string();
        sright = right.get_string();
        result = _strlike((char*)sleft.c_str(),(char*)sright.c_str(),0);
        break;
    }
    case NOT_LIKE:{
        sleft = left.get_string();
        sright = right.get_string();
        result = !_strlike((char*)sleft.c_str(),(char*)sright.c_str(),0);
        break;
    }
    default: {
        LOG_WARN("unsupported comparison. %d", comp_);
        rc = RC::INTERNAL;
    } break;
    }

    return rc;
}

RC ComparisonExpr::try_get_value(Value &cell) const {
    if (left_->type() == ExprType::VALUE && right_->type() == ExprType::VALUE) {
        ValueExpr *left_value_expr = static_cast<ValueExpr *>(left_.get());
        ValueExpr *right_value_expr = static_cast<ValueExpr *>(right_.get());
        const Value &left_cell = left_value_expr->get_value();
        const Value &right_cell = right_value_expr->get_value();

        bool value = false;
        RC rc = compare_value(left_cell, right_cell, value);
        if (rc != RC::SUCCESS) {
            LOG_WARN("failed to compare tuple cells. rc=%s", strrc(rc));
        } else {
            cell.set_boolean(value);
        }
        return rc;
    }

    return RC::INVALID_ARGUMENT;
}

RC ComparisonExpr::get_value(const Tuple &tuple, Value &value) const {
    Value left_value;
    Value right_value;

    RC rc = left_->get_value(tuple, left_value);
    if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
        return rc;
    }
    rc = right_->get_value(tuple, right_value);
    if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
        return rc;
    }

    bool bool_value = false;

    rc = compare_value(left_value, right_value, bool_value);
    if (rc == RC::SUCCESS) {
        value.set_boolean(bool_value);
    }
    return rc;
}

////////////////////////////////////////////////////////////////////////////////
ConjunctionExpr::ConjunctionExpr(Type type, vector<unique_ptr<Expression>> &children) :
        conjunction_type_(type), children_(std::move(children)) {}

RC ConjunctionExpr::get_value(const Tuple &tuple, Value &value) const {
    RC rc = RC::SUCCESS;
    if (children_.empty()) {
        value.set_boolean(true);
        return rc;
    }

    Value tmp_value;
    for (const unique_ptr<Expression> &expr : children_) {
        rc = expr->get_value(tuple, tmp_value);
        if (rc != RC::SUCCESS) {
            LOG_WARN("failed to get value by child expression. rc=%s", strrc(rc));
            return rc;
        }
        bool bool_value = tmp_value.get_boolean();
        if ((conjunction_type_ == Type::AND && !bool_value) || (conjunction_type_ == Type::OR && bool_value)) {
            value.set_boolean(bool_value);
            return rc;
        }
    }

    bool default_value = (conjunction_type_ == Type::AND);
    value.set_boolean(default_value);
    return rc;
}

////////////////////////////////////////////////////////////////////////////////

ArithmeticExpr::ArithmeticExpr(ArithmeticExpr::Type type, Expression *left, Expression *right) :
        arithmetic_type_(type), left_(left), right_(right) {}
ArithmeticExpr::ArithmeticExpr(ArithmeticExpr::Type type, unique_ptr<Expression> left, unique_ptr<Expression> right) :
        arithmetic_type_(type), left_(std::move(left)), right_(std::move(right)) {}

AttrType ArithmeticExpr::value_type() const {
    if (!right_) {
        return left_->value_type();
    }

    if (left_->value_type() == AttrType::INTS && right_->value_type() == AttrType::INTS &&
            arithmetic_type_ != Type::DIV) {
        return AttrType::INTS;
    }

    return AttrType::FLOATS;
}

RC ArithmeticExpr::calc_value(const Value &left_value, const Value &right_value, Value &value) const {
    RC rc = RC::SUCCESS;

    const AttrType target_type = value_type();

    switch (arithmetic_type_) {
    case Type::ADD: {
        if (target_type == AttrType::INTS) {
            value.set_int(left_value.get_int() + right_value.get_int());
        } else {
            value.set_float(left_value.get_float() + right_value.get_float());
        }
    } break;

    case Type::SUB: {
        if (target_type == AttrType::INTS) {
            value.set_int(left_value.get_int() - right_value.get_int());
        } else {
            value.set_float(left_value.get_float() - right_value.get_float());
        }
    } break;

    case Type::MUL: {
        if (target_type == AttrType::INTS) {
            value.set_int(left_value.get_int() * right_value.get_int());
        } else {
            value.set_float(left_value.get_float() * right_value.get_float());
        }
    } break;

    case Type::DIV: {
        if (target_type == AttrType::INTS) {
            if (right_value.get_int() == 0) {
                // NOTE:
                // 设置为整数最大值是不正确的。通常的做法是设置为NULL，但是当前的miniob没有NULL概念，所以这里设置为整数最大值。
                value.set_int(numeric_limits<int>::max());
            } else {
                value.set_int(left_value.get_int() / right_value.get_int());
            }
        } else {
            if (right_value.get_float() > -EPSILON && right_value.get_float() < EPSILON) {
                // NOTE:
                // 设置为浮点数最大值是不正确的。通常的做法是设置为NULL，但是当前的miniob没有NULL概念，所以这里设置为浮点数最大值。
                value.set_float(numeric_limits<float>::max());
            } else {
                value.set_float(left_value.get_float() / right_value.get_float());
            }
        }
    } break;

    case Type::NEGATIVE: {
        if (target_type == AttrType::INTS) {
            value.set_int(-left_value.get_int());
        } else {
            value.set_float(-left_value.get_float());
        }
    } break;

    default: {
        rc = RC::INTERNAL;
        LOG_WARN("unsupported arithmetic type. %d", arithmetic_type_);
    } break;
    }
    return rc;
}

RC ArithmeticExpr::get_value(const Tuple &tuple, Value &value) const {
    RC rc = RC::SUCCESS;

    Value left_value;
    Value right_value;

    rc = left_->get_value(tuple, left_value);
    if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
        return rc;
    }
    rc = right_->get_value(tuple, right_value);
    if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
        return rc;
    }
    return calc_value(left_value, right_value, value);
}

RC ArithmeticExpr::try_get_value(Value &value) const {
    RC rc = RC::SUCCESS;

    Value left_value;
    Value right_value;

    rc = left_->try_get_value(left_value);
    if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
        return rc;
    }

    if (right_) {
        rc = right_->try_get_value(right_value);
        if (rc != RC::SUCCESS) {
            LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
            return rc;
        }
    }

    return calc_value(left_value, right_value, value);
}

FunctionExpr::FunctionExpr(CompFunction type, std::unique_ptr<Expression> left) :
        left_(std::move(left)) {
    function = type;
    bFirst = true;
    this->sum = 0;
    this->num = 0;
}

AttrType FunctionExpr::value_type() const {
    return BOOLEANS;
}

RC FunctionExpr::get_value(const Tuple &tuple, Value &value) const {
    RC rc = RC::SUCCESS;

    rc = left_->get_value(tuple, value);
    if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
        return rc;
    }
    return rc;
}

RC FunctionExpr::try_get_value(Value &value) const {
    RC rc = RC::SUCCESS;
    rc = left_->try_get_value(value);
    if (rc != RC::SUCCESS) {
        LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
        return rc;
    }
    return rc;
}

void FunctionExpr::setValue(Value &value) 
{
    if(this->function == CompFunction::ORDER_FUN)
    {
        this->records.push_back(value);
    }
    if (this->value_.attr_type() == AttrType::UNDEFINED) 
    {
        value_ = value;
        this->sum += value.get_float();
        this->num += 1;
    } else {
        int opC = this->value_.compare(value);
        switch (function) {
        case CompFunction::MAX_FUC: {
            if (opC < 0) {
                this->value_ = value;
            }
            break;
        }
        case CompFunction::MIN_FUC: {
            if (opC > 0) {
                this->value_ = value;
            }
            break;
        }
        case CompFunction::AVG_FUC: {
            this->sum += value.get_float();
            this->num += 1;
            break;
        }
        case CompFunction::COUNT_FUN: {
            this->num += 1;
            break;
        }
        default:
            break;
        }
    }
}

bool FunctionExpr::AddRecord(Value value, Record &rowtuple) 
{
    float vf =  value.get_float();
    MapVecRecords::iterator  itFind;
    itFind = this->mapRecords.find(vf);
    if( itFind == this->mapRecords.end() )
    {
      VecRecords   vec;
      vec.push_back(rowtuple);
      this->mapRecords[vf] = vec;
    }
    else
    {
      itFind->second.push_back(rowtuple);
    }
    return true;
}

