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
// Created by wangyunlai on 2021/6/11.
//

#include "common/defs.h"
#include <algorithm>
#include <string.h>

namespace common {

int compare_int(void *arg1, void *arg2)
{
  int v1 = *(int *)arg1;
  int v2 = *(int *)arg2;
  if (v1 > v2) {
    return 1;
  } else if (v1 < v2) {
    return -1;
  } else {
    return 0;
  }
}

int compare_float(void *arg1, void *arg2)
{
  float v1  = *(float *)arg1;
  float v2  = *(float *)arg2;
  float cmp = v1 - v2;
  if (cmp > EPSILON) {
    return 1;
  }
  if (cmp < -EPSILON) {
    return -1;
  }
  return 0;
}

int compare_string(void *arg1, int arg1_max_length, void *arg2, int arg2_max_length)
{
  const char *s1     = (const char *)arg1;
  const char *s2     = (const char *)arg2;
  int         maxlen = std::min(arg1_max_length, arg2_max_length);
  int         result = strncmp(s1, s2, maxlen);
  if (0 != result) {
    return result;
  }

  if (arg1_max_length > maxlen) {
    return s1[maxlen] - 0;
  }

  if (arg2_max_length > maxlen) {
    return 0 - s2[maxlen];
  }
  return 0;
}

// ##
bool check_date(int y, int m, int d)
{
  static int mon[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  bool       leap  = (y % 400 == 0 || (y % 100 && y % 4 == 0));
  return y > 0 && (m > 0) && (m <= 12) && (d > 0) && (d <= ((m == 2 && leap) ? 1 : 0) + mon[m]);
}
int value_init_date(const char *v, int len)
{
  int y, m, d;
  sscanf(v, "%d-%d-%d", &y, &m, &d);  // not check return value eq 3, lex guarantee
  bool b = check_date(y, m, d);
  if (!b)
    return -1;
  int dv = y * 10000 + m * 100 + d;
  return dv;
}
bool GetYearMonthDay(int v, int &oy, int &om, int &od)
{
  oy = v / 10000;
  om = (v % 10000) / 100;
  od = (v % 100);
  return check_date(oy, om, od);
}

int compare_date(void *arg1, void *arg2)
{
  int v1 = *(int *)arg1;
  int v2 = *(int *)arg2;

  int y1, m1, d1;
  int y2, m2, d2;
  GetYearMonthDay(v1, y1, m1, d1);
  GetYearMonthDay(v2, y2, m2, d2);
  if (y1 > y2) {
    return 1;
  } else if (y1 < y2) {
    return -1;
  } 
  else 
  {
    if(m1 > m2)
    {
      return 1;
    }
    else if(m1 < m2)
    {
      return -1;
    }
    else 
    {
      if(d1 > d2)
      {
        return 1;
      }
      else if(d1 < d2)
      {
        return -1;
      }
    }
    return 0;
  }
}

}  // namespace common
