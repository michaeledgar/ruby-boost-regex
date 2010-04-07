#include <boost/regex.hpp>
#include <iostream>
#include <string>
#include <exception>
#include "ruby.h"
#include "re.h"

static VALUE rb_mBoost;
static VALUE rb_cBoostRegexp;

static VALUE rb_kRegexpMultiline;
static VALUE rb_kRegexpIgnorecase;
static VALUE rb_kRegexpExtended;

///////// imported from re.c

#define RE_TALLOC(n,t)  ((t*)alloca((n)*sizeof(t)))
#define TMALLOC(n,t)    ((t*)xmalloc((n)*sizeof(t)))
#define TREALLOC(s,n,t) (s=((t*)xrealloc(s,(n)*sizeof(t))))
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

static void
init_regs(struct re_registers *regs, unsigned int num_regs)
{
  int i;

  regs->num_regs = num_regs;
  if (num_regs < RE_NREGS)
    num_regs = RE_NREGS;

  if (regs->allocated == 0) {
    regs->beg = TMALLOC(num_regs, int);
    regs->end = TMALLOC(num_regs, int);
    regs->allocated = num_regs;
  }
  else if (regs->allocated < num_regs) {
    TREALLOC(regs->beg, num_regs, int);
    TREALLOC(regs->end, num_regs, int);
    regs->allocated = num_regs;
  }
  for (i=0; i<num_regs; i++) {
    regs->beg[i] = regs->end[i] = -1;
  }
}

void
re_copy_registers(struct re_registers *regs1, struct re_registers *regs2)
{
  int i;

  if (regs1 == regs2) return;
  if (regs1->allocated == 0) {
    regs1->beg = TMALLOC(regs2->num_regs, int);
    regs1->end = TMALLOC(regs2->num_regs, int);
    regs1->allocated = regs2->num_regs;
  }
  else if (regs1->allocated < regs2->num_regs) {
    TREALLOC(regs1->beg, regs2->num_regs, int);
    TREALLOC(regs1->end, regs2->num_regs, int);
    regs1->allocated = regs2->num_regs;
  }
  for (i=0; i<regs2->num_regs; i++) {
    regs1->beg[i] = regs2->beg[i];
    regs1->end[i] = regs2->end[i];
  }
  regs1->num_regs = regs2->num_regs;
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
VALUE br_init(int argc, VALUE *argv, VALUE self) {
    VALUE reg_to_convert, flags;
    boost::regex *reg;
    VALUE str;
    int newflags, oldflagsint;
    rb_scan_args(argc, argv, "11", &reg_to_convert, &flags);

    reg = get_br_from_value(self);
    if (NIL_P(flags)) {
        flags = UINT2NUM(boost::regex_constants::normal);
    }
    try {
        if (TYPE(reg_to_convert) == T_REGEXP) {
            str = rb_funcall(reg_to_convert, rb_intern("source"), 0);
            // calculate the flags to use
            newflags = NUM2UINT(flags);
            VALUE oldflags = rb_funcall(reg_to_convert, rb_intern("options"), 0);
            int oldflagsint = FIX2INT(oldflags);
            // convert ruby regexp flags to boost regex flags
            if (oldflagsint & FIX2INT(rb_kRegexpIgnorecase))
                newflags |= boost::regex_constants::icase;
            if (oldflagsint & FIX2INT(rb_kRegexpMultiline))
                newflags |= boost::regex_constants::mod_s;
            if (oldflagsint & FIX2INT(rb_kRegexpExtended))
                newflags |= boost::regex_constants::mod_x;
            // convert back to fixnum
            flags = UINT2NUM(newflags);
        } else {
            str = rb_convert_type(reg_to_convert, T_STRING, "String", "to_s");
        }
        *reg = boost::regex(RSTRING_PTR(str), FIX2UINT(flags));
    } catch (boost::regex_error& exc) {
        // C++ exceptions have to be re-raised as ruby
        rb_raise(rb_eArgError, "Invalid regular expression");
    }
}

static VALUE get_backref_for_modification() {
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
    return match;
}

static void
fill_regs_from_smatch(std::string::const_iterator first, std::string::const_iterator last, struct re_registers *regs, boost::smatch matches) {
    init_regs(regs, matches.size());
    regs->beg[0] = matches[0].first - first;
    regs->end[0] = matches[0].second - first;
    
    for (int idx = 1; idx <= matches.size(); idx++) {
        if (!matches[idx].matched) {
            regs->beg[idx] = regs->end[idx] = -1;
        } else {
            regs->beg[idx] = matches[idx].first - first;
            regs->end[idx] = matches[idx].second - first;
        }
    }
}

/**
 * General matcher method that re-raises exception as a Ruby exception.  Gotta use this. sorry.
 */
static bool 
br_reg_match_iters(VALUE str, std::string::const_iterator start, std::string::const_iterator stop, boost::smatch& matches, boost::regex reg)
{
    static struct re_registers regs;
    try {
        if (boost::regex_search(start, stop, matches, reg)) {
            VALUE match = get_backref_for_modification();
            RMATCH(match)->str = rb_str_dup(str);
            fill_regs_from_smatch(start, stop, &regs, matches);
            re_copy_registers(RMATCH(match)->regs, &regs);
            rb_backref_set(match);
            return true;
        } else {
            rb_backref_set(Qnil);
            return false;
        }
    } catch (std::runtime_error& err) {
        rb_raise(rb_eRegexpError, "Ran out of resources while matching a Boost Regexp.");
    }
}

static int 
br_reg_search(VALUE self, VALUE str) {
    boost::regex reg = *get_br_from_value(self);
    std::string input = StringValuePtr(str);
    
    std::string::const_iterator start, end;
    start = input.begin();
    end = input.end();
    
    boost::smatch matches;
    if (br_reg_match_iters(str, start, end, matches, reg)) {
        return matches[0].first - start;
    } else {
        return -1;
    }
}

static VALUE 
br_reg_do_match(VALUE self, VALUE str) {
    boost::regex reg = *get_br_from_value(self);
    std::string input = StringValuePtr(str);
    
    std::string::const_iterator start, end;
    start = input.begin();
    end = input.end();
    
    boost::smatch matches;
    if (br_reg_match_iters(str, start, end, matches, reg)) {
        return rb_backref_get();
    } else {
        return Qnil;
    }
}

static VALUE br_eql(VALUE self, VALUE other) {
    if (!rb_obj_is_kind_of(other, rb_cBoostRegexp)) {
        return Qfalse;
    }
    boost::regex a, b;
    a = *get_br_from_value(self);
    b = *get_br_from_value(other);
    return (a == b) ? Qtrue : Qfalse;
}

static VALUE br_match_operator(VALUE self, VALUE str) {
    int start = br_reg_search(self, str);
    if (start < 0) {
        return Qnil;
    }
    return INT2FIX(start);
}

static VALUE br_match_eqq_operator(VALUE self, VALUE str) {
    int start = br_reg_search(self, str);
    if (start < 0) {
        return Qfalse;
    } else {
        return Qtrue;
    }
}

static VALUE br_options(VALUE self) {
    boost::regex reg = *get_br_from_value(self);
    return UINT2NUM(reg.flags());
}

static VALUE br_source(VALUE self) {
    boost::regex reg = *get_br_from_value(self);
    std::string result = reg.str();
    return rb_str_new(result.c_str(), result.size());
}

static VALUE br_tilde_operator(VALUE self) {
    VALUE target = rb_gv_get("$_");
    return br_match_operator(self, target);
}

extern "C" {
    VALUE Init_BoostRegexHook()
    {
        rb_eRegexpError = rb_define_class("RegexpError", rb_eStandardError);
        
        rb_cMatch = rb_const_get(rb_cObject, rb_intern("MatchData"));
        rb_kRegexpMultiline  = rb_const_get(rb_cRegexp, rb_intern("MULTILINE"));
        rb_kRegexpIgnorecase = rb_const_get(rb_cRegexp, rb_intern("IGNORECASE"));
        rb_kRegexpExtended   = rb_const_get(rb_cRegexp, rb_intern("EXTENDED"));
        
        rb_mBoost = rb_define_module("Boost");
        rb_cBoostRegexp = rb_define_class_under(rb_mBoost, "Regexp", rb_cObject);
        
        rb_define_alloc_func(rb_cBoostRegexp, br_alloc);
        rb_define_method(rb_cBoostRegexp, "initialize", RUBY_METHOD_FUNC(br_init), -1);
        rb_define_method(rb_cBoostRegexp, "=~", RUBY_METHOD_FUNC(br_match_operator), 1);
        rb_define_method(rb_cBoostRegexp, "===", RUBY_METHOD_FUNC(br_match_eqq_operator), 1);
        rb_define_method(rb_cBoostRegexp, "source", RUBY_METHOD_FUNC(br_source), 0);
        rb_define_method(rb_cBoostRegexp, "match", RUBY_METHOD_FUNC(br_reg_do_match), 1);
        rb_define_method(rb_cBoostRegexp, "options", RUBY_METHOD_FUNC(br_options), 0);
        rb_define_method(rb_cBoostRegexp, "~", RUBY_METHOD_FUNC(br_tilde_operator), 0);
        rb_define_method(rb_cBoostRegexp, "==", RUBY_METHOD_FUNC(br_eql), 1);
        rb_define_method(rb_cBoostRegexp, "eql?", RUBY_METHOD_FUNC(br_eql), 1);
        
        rb_define_const(rb_cBoostRegexp, "NORMAL", UINT2NUM(boost::regex_constants::normal));
        rb_define_const(rb_cBoostRegexp, "ECMASCRIPT", UINT2NUM(boost::regex_constants::ECMAScript));
        rb_define_const(rb_cBoostRegexp, "JAVASCRIPT", UINT2NUM(boost::regex_constants::JavaScript));
        rb_define_const(rb_cBoostRegexp, "JSCRIPT", UINT2NUM(boost::regex_constants::JScript));
        rb_define_const(rb_cBoostRegexp, "PERL", UINT2NUM(boost::regex_constants::perl));
        rb_define_const(rb_cBoostRegexp, "BASIC", UINT2NUM(boost::regex_constants::basic));
        rb_define_const(rb_cBoostRegexp, "SED", UINT2NUM(boost::regex_constants::sed));
        rb_define_const(rb_cBoostRegexp, "EXTENDED", UINT2NUM(boost::regex_constants::extended));
        rb_define_const(rb_cBoostRegexp, "AWK", UINT2NUM(boost::regex_constants::awk));
        rb_define_const(rb_cBoostRegexp, "GREP", UINT2NUM(boost::regex_constants::grep));
        rb_define_const(rb_cBoostRegexp, "EGREP", UINT2NUM(boost::regex_constants::egrep));
        rb_define_const(rb_cBoostRegexp, "IGNORECASE", UINT2NUM(boost::regex_constants::icase));
        rb_define_const(rb_cBoostRegexp, "NO_SUBS", UINT2NUM(boost::regex_constants::nosubs));
        rb_define_const(rb_cBoostRegexp, "OPTIMIZE", UINT2NUM(boost::regex_constants::optimize));
        rb_define_const(rb_cBoostRegexp, "COLLATE", UINT2NUM(boost::regex_constants::collate));
        
        rb_define_const(rb_cBoostRegexp, "NO_EXCEPTIONS", UINT2NUM(boost::regex_constants::no_except));
        rb_define_const(rb_cBoostRegexp, "SAVE_SUBEXPRESSION_LOCS", UINT2NUM(boost::regex_constants::save_subexpression_location));
        
        rb_define_const(rb_cBoostRegexp, "NO_MOD_M", UINT2NUM(boost::regex_constants::no_mod_m));
        rb_define_const(rb_cBoostRegexp, "DOTS_NEVER_NEWLINES", UINT2NUM(boost::regex_constants::no_mod_s));
        rb_define_const(rb_cBoostRegexp, "DOTS_MATCH_NEWLINES", UINT2NUM(boost::regex_constants::mod_s));
        rb_define_const(rb_cBoostRegexp, "IGNORE_WHITESPACE", UINT2NUM(boost::regex_constants::mod_x));
        rb_define_const(rb_cBoostRegexp, "NO_EMPTY_EXPRESSIONS", UINT2NUM(boost::regex_constants::no_empty_expressions));
        
        rb_define_const(rb_cBoostRegexp, "NO_CHAR_CLASSES", UINT2NUM(boost::regex_constants::no_char_classes));
        rb_define_const(rb_cBoostRegexp, "NO_ESCAPE_IN_LISTS", UINT2NUM(boost::regex_constants::no_escape_in_lists));
        rb_define_const(rb_cBoostRegexp, "NO_INTERVALS", UINT2NUM(boost::regex_constants::no_intervals));
        rb_define_const(rb_cBoostRegexp, "BK_PLUS_QM", UINT2NUM(boost::regex_constants::bk_plus_qm));
        rb_define_const(rb_cBoostRegexp, "BK_VBAR", UINT2NUM(boost::regex_constants::bk_vbar));
        rb_define_const(rb_cBoostRegexp, "LITERAL", UINT2NUM(boost::regex_constants::literal));
    }
}