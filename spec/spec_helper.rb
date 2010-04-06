$LOAD_PATH.unshift(File.dirname(__FILE__))
$LOAD_PATH.unshift(File.join(File.dirname(__FILE__), '..', 'lib'))
require 'ruby-boost-regex'
require 'ruby-boost-regex/BoostRegexHook'
require 'spec'
require 'spec/autorun'

Spec::Matchers.define :have_flag do |flag|
  match do |bitmask|
    bitmask & flag > 0
  end
end

Spec::Matchers.define :match do |string|
  match do |regexp|
    regexp.match(string) != nil
  end
end

Spec::Runner.configure do |config|
  
end
