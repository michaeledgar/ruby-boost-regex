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
    Normal regex  17.190000   0.020000  17.210000 ( 17.278168)
    Boost regex   12.120000   0.030000  12.150000 ( 12.213959)
    -------------------------------------- total: 29.360000sec
    
                       user     system      total        real
    Normal regex  17.050000   0.020000  17.070000 ( 17.082539)
    Boost regex   12.000000   0.030000  12.030000 ( 12.040932)
    
    Failing to match a phone number in a big string of text
    =======================================================
    Rehearsal ------------------------------------------------
    Normal regex   0.070000   0.000000   0.070000 (  0.070072)
    Boost regex    0.030000   0.000000   0.030000 (  0.034858)
    --------------------------------------- total: 0.100000sec
    
                       user     system      total        real
    Normal regex   0.070000   0.000000   0.070000 (  0.070087)
    Boost regex    0.040000   0.000000   0.040000 (  0.035052)


## Usage

Don't have it as a gem yet. Sorry! But you could do this in theory:

    r = Boost::Regexp.new("(\\d{3})-(\\d{3})-(\\d{4})")
    r =~ "555-123-4567"
    p $1 # ==> "555"
    matches = r.match("123-456-7890")
    p matches[2] # ==> "456"


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
