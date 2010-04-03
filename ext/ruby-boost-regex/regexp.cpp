#include <boost/regex.hpp>
#include <iostream>
#include <string>
#include <exception>
#include "ruby.h"
#include "re.h"

static VALUE rb_mBoost;
static VALUE rb_cBoostRegexp;

///////// imported from re.c

#define MATCH_BUSY FL_USER2
static VALUE match_alloc(VALUE klass)
{
    NEWOBJ(match, struct RMatch);
    OBJSETUP(match, klass, T_MATCH);

    match->str = 0;
    match->regs = 0;
    match->regs = ALLOC(struct re_registers);
    MEMZERO(match->regs, struct re_registers, 1);

    return (VALUE)match;
}

/////////////////////////////

// extracts the boost regex using Data_Get_Struct
boost::regex *get_br_from_value(VALUE self) {
    boost::regex *p;
    Data_Get_Struct(self, boost::regex, p);
    return p;
}

/**
 * Frees the boost regex.
 */
static void br_free(void *p) {
    delete (boost::regex *)p;
}

/**
 * Allocates a new Boost::Regexp by allocating heap space via new for boost::regex.
 * Does not initialize the regexp.
 */
static VALUE br_alloc(VALUE klass) {
    boost::regex *reg;
    VALUE object;
    
    reg = new boost::regex;
    object = Data_Wrap_Struct(klass, NULL, br_free, reg);
    return object;
}

/**
 * Initializes a regexp with a pattern and options.
 * The boost regex is already allocated so we just call the constructor here.
 */
VALUE br_init(VALUE self, VALUE obj) {
    boost::regex *reg = get_br_from_value(self);
    VALUE str = rb_convert_type(obj, T_STRING, "String", "to_s");
    try {
        *reg = boost::regex(RSTRING_PTR(str));
    } catch (boost::regex_error& exc) {
        // C++ exceptions have to be re-raised as ruby
        rb_raise(rb_eArgError, "Invalid regular expression");
    }
}

/**
 * General matcher method that re-raises exception as a Ruby exception.  Gotta use this. sorry.
 */
bool br_reg_match_iters(std::string::const_iterator start, std::string::const_iterator stop, boost::smatch& matches, boost::regex reg)
{
    try {
        if (boost::regex_search(start, stop, matches, reg)) {
            VALUE match;
            match = rb_backref_get();
            if (NIL_P(match) || FL_TEST(match, MATCH_BUSY)) {
                match = match_alloc(rb_cMatch);
            }
            else {
                if (rb_safe_level() >= 3) 
                    OBJ_TAINT(match);
                else
                    FL_UNSET(match, FL_TAINT);
            }
            return true;
        } else {
            return false;
        }
    } catch (std::runtime_error& err) {
        rb_raise(rb_eRegexpError, "Ran out of resources while matching a Boost Regexp.");
    }
}

static unsigned int br_reg_search(VALUE self, VALUE str) {
    boost::regex reg = *get_br_from_value(self);
    std::string input = StringValuePtr(str);
    
    std::string::const_iterator start, end;
    start = input.begin();
    end = input.end();
    
    boost::smatch matches;
    if (br_reg_match_iters(start, end, matches, reg)) {
        return matches[0].first - start;
    } else {
        return -1;
    }
}

static VALUE br_match_operator(VALUE self, VALUE str) {
    unsigned int start = br_reg_search(self, str);
    if (start < 0) {
        return Qnil;
    }
    return INT2FIX(start);
}

static VALUE br_match_eqq_operator(VALUE self, VALUE str) {
    unsigned int start = br_reg_search(self, str);
    if (start < 0) {
        return Qfalse;
    } else {
        return Qtrue;
    }
}

static VALUE br_inspect(VALUE self) {
    boost::regex reg = *get_br_from_value(self);
    std::string slash = "/";
    std::string finished = (slash + reg.str() + slash).c_str();
    return rb_str_new(finished.c_str(), finished.size());
}

static VALUE br_source(VALUE self) {
    boost::regex reg = *get_br_from_value(self);
    std::string result = reg.str();
    return rb_str_new(result.c_str(), result.size());
}



extern "C" {
    VALUE Init_BoostRegexHook()
    {
        rb_eRegexpError = rb_define_class("RegexpError", rb_eStandardError);
        rb_cMatch = rb_const_get(rb_cObject, rb_intern("MatchData"));
        rb_mBoost = rb_define_module("Boost");
        rb_cBoostRegexp = rb_define_class_under(rb_mBoost, "Regexp", rb_cObject);
        rb_define_alloc_func(rb_cBoostRegexp, br_alloc);
        rb_define_method(rb_cBoostRegexp, "initialize", RUBY_METHOD_FUNC(br_init), 1);
        rb_define_method(rb_cBoostRegexp, "=~", RUBY_METHOD_FUNC(br_match_operator), 1);
        rb_define_method(rb_cBoostRegexp, "===", RUBY_METHOD_FUNC(br_match_eqq_operator), 1);
        rb_define_method(rb_cBoostRegexp, "inspect", RUBY_METHOD_FUNC(br_inspect), 0);
        rb_define_method(rb_cBoostRegexp, "source", RUBY_METHOD_FUNC(br_source), 0);
        
    }
}