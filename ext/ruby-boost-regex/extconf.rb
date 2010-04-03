require 'mkmf'

have_library("stdc++")
have_library("boost_regex")
create_makefile('ruby-boost-regex/BoostRegexHook')