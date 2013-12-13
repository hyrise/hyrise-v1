#include "expr_PCScan.h"

#include "access/expressions/ExpressionRegistration.h"

namespace hyrise { namespace access {
namespace { auto reg_exp_pc_scan_f1_eq_int = Expressions::add< PCScan_F1_OP_TYPE<hyrise_int_t, std::equal_to<hyrise_int_t>> >("hyrise::PCScan_F1_EQ_INT"); }
}}
