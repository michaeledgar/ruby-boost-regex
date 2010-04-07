# ruby-boost-regex

Wraps the Boost.Regex library to provide Boost regexes. Why? Why not!
(Plus, it's worth seeing if it's faster.)

I'm trying to match the API to be at least completely compatible with
normal `Regexp`s, and then add Boost features. So far...

## Features

* Matches set global special variables like $`, $1-$9, $&, and so on
* Supports `=~` operator, `===` operator (for `case...when`)
* Supports `#match` method as in normal Regexps, returning the same
  type of object (`MatchData`).
* Spiffy monkey patch (see below)
  
## Cool monkey patch

So monkey patching is bad, right? Right. And rubyists monkey patch all the time,
which makes us bad people.  Well, this module adds a new, alternate regex, but
Ruby has a syntax for regexes already:

    reg = /\d{3}/
    
This is nice because we don't have to escape the backslashes, plus it looks really
nice with syntax highlighting.  With these boost regexes, you'll either have to do

    reg = Boost::Regexp.new("\\d{3}")
    # or
    reg = Boost::Regexp.new(/\d{3}/)
    
Why do all that typing? We have a literal regex syntax!  But it creates normal regexes.
So, we have a compromise:

    Boost::Regexp.enable_monkey_patch! # only have to do this once
    reg = /\d{3}/.boost!
    reg.class # ==> Boost::Regexp
    
Cool, eh?

Of course, Boost gives us lots of crazy flags:

    reg = /abc(def)/.boost!(Boost::Regexp::NO_SUBS & Boost::Regexp::IGNORECASE)
    reg =~ "zzzABCDEF" # ==> 3
    puts $1  # ==> nil
    
## Benchmarks

I adapted the [ruby-benchmark-suite](http://github.com/acangiano/ruby-benchmark-suite)'s bm\_regex\_dna
benchmark, which is used in the Computer Language Shootout.  However, it uses str.scan(), which will fail
if confronted with a non-standard Regexp.  So I had to write my own, which is less optimized.  The
[benchmark code](http://github.com/michaeledgar/ruby-boost-regex/blob/master/benchmark/benchmark.rb) is
free to be scrutinized!  It uses the standard benchmark module because lifting the spiffy one from ruby-benchmark-suite
proved to be too much of a hassle for now.

Anyway, here's some results:

    DNA-Matching (Computer Language Shootout)
    =========================================
    Rehearsal ------------------------------------------------
    Normal regex  17.240000   0.050000  17.290000 ( 17.353051)
    Oniguruma     16.300000   0.030000  16.330000 ( 16.384928)
    Boost regex   11.400000   0.040000  11.440000 ( 11.489252)
    -------------------------------------- total: 45.060000sec
    
                       user     system      total        real
    Normal regex  17.190000   0.030000  17.220000 ( 17.273140)
    Oniguruma     16.220000   0.040000  16.260000 ( 16.325460)
    Boost regex   11.330000   0.030000  11.360000 ( 11.402222)
    
    Failing to match a phone number in a big string of text
    =======================================================
    Rehearsal ------------------------------------------------
    Normal regex   0.070000   0.000000   0.070000 (  0.072128)
    Oniguruma      0.040000   0.000000   0.040000 (  0.043422)
    Boost regex    0.040000   0.000000   0.040000 (  0.034708)
    --------------------------------------- total: 0.150000sec
    
                       user     system      total        real
    Normal regex   0.070000   0.000000   0.070000 (  0.071984)
    Oniguruma      0.040000   0.000000   0.040000 (  0.044686)
    Boost regex    0.030000   0.000000   0.030000 (  0.036421)

## Usage

Install the gem, use as follows:

    require 'ruby-boost-regex'

    r = Boost::Regexp.new("(\\d{3})-(\\d{3})-(\\d{4})")
    r =~ "555-123-4567"
    p $1 # ==> "555"
    matches = r.match("123-456-7890")
    p matches[2] # ==> "456"
    
    Boost::Regex.enable_monkey_patch!
    
    r = /hello|world/i.boost!
    r =~ "i'm Mike. Hello!" #==> 10
    


## Installation

    gem install ruby-boost-regex

## Note on Patches/Pull Requests
 
* Fork the project.
* Make your feature addition or bug fix.
* Add tests for it. This is important so I don't break it in a
  future version unintentionally.
* Commit, do not mess with rakefile, version, or history.
  (if you want to have your own version, that is fine but
   bump version in a commit by itself I can ignore when I pull)
* Send me a pull request. Bonus points for topic branches.

## Copyright

Copyright (c) 2010 Michael Edgar. See LICENSE for details.
