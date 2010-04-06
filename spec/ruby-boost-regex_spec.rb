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