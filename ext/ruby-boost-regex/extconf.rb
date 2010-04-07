require 'mkmf'

dir_config("boost")
have_library("stdc++")
have_library("boost_regex")
if RUBY_VERSION =~ /1.9/ then  
    $CPPFLAGS += " -DRUBY_19"  
end
create_makefile('ruby-boost-regex/BoostRegexHook')