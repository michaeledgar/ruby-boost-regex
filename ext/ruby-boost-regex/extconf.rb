require 'mkmf'

dir_config("boost")
have_library("stdc++")
have_library("boost_regex")
create_makefile('ruby-boost-regex/BoostRegexHook')