require File.expand_path(File.dirname(__FILE__) + '/spec_helper')

describe Boost::Regexp do
  it "creates wrapped regexes" do
    reg = Boost::Regexp.new("\\d{3}")
    reg.should be_a(Boost::Regexp)
  end
  
  it "looks like a regex when inspected" do
    reg = Boost::Regexp.new("\\d{3}")
    reg.inspect.should == "/\\d{3}/"
  end
  
  it "can be initialized with a string" do
    reg = Boost::Regexp.new("abc")
    reg.should match("jklabcdef")
  end
  
  it "can be initialized with a regexp" do
    reg = Boost::Regexp.new(/abc/)
    reg.should match("jklabcdef")
  end
end

describe Boost::Regexp, "#source" do
  it "returns the source string of the regexp" do
    Boost::Regexp.new('(\d{3})-(\d{2})-(\d{4})').source.should == '(\d{3})-(\d{2})-(\d{4})'
  end
end

describe Boost::Regexp, "#options" do
  it "returns the flags used by the regexp" do
    reg = Boost::Regexp.new("ab  cd", Boost::Regexp::IGNORE_WHITESPACE)
    reg.options.should have_flag(Boost::Regexp::IGNORE_WHITESPACE)
    reg.options.should_not have_flag(Boost::Regexp::LITERAL)
  end
end

describe Boost::Regexp, "#=~" do
  it "returns the position of the match" do
    result = Boost::Regexp.new("abcd") =~ "zxabcdefg"
    result.should == 2
  end
  
  it "returns nil on no match" do
    result = Boost::Regexp.new("abcd") =~ "aj3ioqh"
    result.should be_nil
  end
  
  it "sets the special match variables on a match" do
    Boost::Regexp.new("abcd") =~ "xyzabcdef"
    $`.should == "xyz"
  end
  
  it "sets the special match variables to nil when a match fails" do
    Boost::Regexp.new("abcd") =~ "uqioer"
    $'.should be_nil
  end
end

describe Boost::Regexp, "#===" do
  it "returns true when a match succeeds" do
    result = Boost::Regexp.new("abcd") === "uioabcdefg"
    result.should be_true
  end
  
  it "returns false when a match fails" do
    result = Boost::Regexp.new("abcd") === "uiot"
    result.should be_false
  end
  
  it "sets the special match variables when a match succeeds" do
    Boost::Regexp.new("abcd") === "xyzabcdef"
    $`.should == "xyz"
  end
  
  it "sets the special match variables to nil when a match fails" do
    Boost::Regexp.new("abcd") === "uqioer"
    $'.should be_nil
  end
end

describe Boost::Regexp, "#~" do
  it "matches the regex against $_" do
    $_ = "123abcdefg"
    result = ~Boost::Regexp.new("abcd")
    result.should == 3
  end
  
  it "negatively matches against $_" do
    $_ = "12345"
    result = ~Boost::Regexp.new("efgh")
    result.should be_nil
  end
  
  it "sets the special match variables when a match succeeds" do
    $_ = "xyzabcdef"
    ~Boost::Regexp.new("abcd")
    $`.should == "xyz"
  end
  
  it "sets the special match variables to nil when a match fails" do
    $_ = "uqioer"
    ~Boost::Regexp.new("abcd")
    $'.should be_nil
  end
end

describe Boost::Regexp, "#match" do
  it "returns a MatchData object" do
    Boost::Regexp.new('\d{3}').match("abc1234def").should be_a(MatchData)
  end
  
  it "returns $~" do
    result = Boost::Regexp.new('\d{3}').match("abc1234def")
    result.should == $~
  end
  
  it "provides captured subexpressions" do
    result = Boost::Regexp.new('(\d{3})-(\d{3})-(\d{4})').match("abc 555-123-4567 def")
    result[0].should == "555-123-4567"
    result[1].should == "555"
    result[2].should == "123"
    result[3].should == "4567"
  end
  
  it "sets the value of Regexp#last_match" do
    result = Boost::Regexp.new('\d{3}').match("abc123def")
    result.should == Regexp.last_match
  end
  
  it "sets the special match variables when a match succeeds" do
    Boost::Regexp.new("abcd").match "xyzabcdef"
    $`.should == "xyz"
  end
  
  it "sets the special match variables to nil when a match fails" do
    Boost::Regexp.new("abcd").match "uqioer"
    $'.should be_nil
  end
end

describe Boost::Regexp, "flags" do
  it "matches with a case-insensitivity flag" do
    Boost::Regexp.new('abc', Boost::Regexp::IGNORECASE).should match "DEFABCJKL"
  end
  
  it "responds correctly to casefold?" do
    Boost::Regexp.new('abc', Boost::Regexp::IGNORECASE).casefold?.should be_true
    Boost::Regexp.new('abc').casefold?.should be_false
  end
  
  it "ignores subexpressions when NO_SUBS is on" do
    result = Boost::Regexp.new('abc(def)', Boost::Regexp::NO_SUBS).match("abcdef")
    result[1].should be_nil
  end
  
  it "allows subexpressions when NO_SUBS is off" do
    result = Boost::Regexp.new('abc(def)').match("abcdef")
    result[1].should == "def"
  end
  
  it "raises an exception when given an invalid regexp and NO_EXCEPTIONS is off" do
    lambda { Boost::Regexp.new("[[:alnum]]")}.should raise_exception(ArgumentError)
  end
  
  it "does not raise an exception when given an invalid regexp and NO_EXCEPTIONS is on" do
    lambda { Boost::Regexp.new("[[:alnum]]", Boost::Regexp::NO_EXCEPTIONS)}.should_not raise_exception(ArgumentError)
  end
  
  it "ignores whitespace when IGNORE_WHITESPACE is set" do
    Boost::Regexp.new("ab cd", Boost::Regexp::IGNORE_WHITESPACE).should match("abcd")
  end
  
  it "doesn't ignore whitespace when IGNORE_WHITESPACE is off" do
    Boost::Regexp.new("ab cd").should_not match "abcd"
  end
  
  it "allows dots to match newlines when DOTS_MATCH_NEWLINES is on" do
    Boost::Regexp.new("abc.def", Boost::Regexp::DOTS_MATCH_NEWLINES).should match "abc\ndef"
  end
  
  it "doesn't allow dots to match newlines when DOTS_NEVER_NEWLINES is on" do
    Boost::Regexp.new("abc.def", Boost::Regexp::DOTS_NEVER_NEWLINES).should_not match "abc\ndef"
  end
end

describe Boost::Regexp, "#enable_monkey_patch!" do
  it "adds a boost! method to normal Regexps" do
    Boost::Regexp.enable_monkey_patch!
    /hello/.should respond_to(:boost!)
  end
  
  it "adds a boost! method that converts Regexps to Boost::Regexps" do
    Boost::Regexp.enable_monkey_patch!
    reg = /hello/.boost!
    reg.should be_a(Boost::Regexp)
  end
  
  it "allows flags to be passed into the added boost! method" do
    Boost::Regexp.enable_monkey_patch!
    reg = /hello/.boost!(Boost::Regexp::IGNORECASE)
    reg.casefold?.should be_true
  end
end