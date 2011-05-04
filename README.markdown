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
* Ruby 1.9 Support
* Near-perfect drop-in for built-in Regexes.

## Problems

* NO Unicode support. Sorry.

## Global Variables

If you use Ruby's regexes, you probably use the handy-dandy global variables that
come along with them, especially for quickly-written scripts.  I'm talking about
our buddies, `$1, $2, $3`, `$\``, `$&`, `$~`. This gem makes sure to set those variables
so that these global variables still work.

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

Anyway, here's some results.  2.4Ghz Intel Core 2 Duo, 4GB 1067 Mhz DDR3, Macbook Pro. Ruby 1.8.7 (2008-08-11 patchlevel 72) [universal-darwin10.0]. Oniguruma is the Oniguruma gem for 1.8.x ruby, not the one bundled with Ruby 1.9+. I ran this a few
times with differing loads, and they always roughly come out to the same result. YMMV.

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
    Normal regex   7.090000   0.160000   7.250000 (  8.025125)
    Oniguruma      4.270000   0.100000   4.370000 (  4.831689)
    Boost regex    3.450000   0.070000   3.520000 (  3.959367)
    -------------------------------------- total: 15.140000sec

                       user     system      total        real
    Normal regex   7.080000   0.130000   7.210000 (  7.979815)
    Oniguruma      4.230000   0.100000   4.330000 (  4.772939)
    Boost regex    3.400000   0.080000   3.480000 (  3.846438)
    
Ruby 1.9 version:

    DNA-Matching (Computer Language Shootout)
    =========================================
    Rehearsal --------------------------------------------------
    1.9 Ruby regex  17.440000   0.030000  17.470000 ( 17.523726)
    Boost regex      9.750000   0.010000   9.760000 (  9.757300)
    ---------------------------------------- total: 27.230000sec
    
                         user     system      total        real
    1.9 Ruby regex  17.420000   0.020000  17.440000 ( 17.434514)
    Boost regex      9.750000   0.010000   9.760000 (  9.755085)
    
    Failing to match a phone number in a big string of text
    =======================================================
    Rehearsal --------------------------------------------------
    1.9 Ruby regex  15.650000   0.310000  15.960000 ( 16.069783)
    Boost regex     38.230000   0.810000  39.040000 ( 40.099560)
    ---------------------------------------- total: 55.000000sec

                         user     system      total        real
    1.9 Ruby regex  15.640000   0.200000  15.840000 ( 16.079149)
    Boost regex     37.950000   0.720000  38.670000 ( 39.023587)

Very interesting - 1.9 seems to have gotten faster at the "match a phone number in a big block of text" benchmark.  Boost is even faster,
though, and Oniguruma integrated with 1.9 is back at the old 1.9 speed. For some reason Boost is much slower at failed searches: this is definitely worth investigating.  I'm hoping to add some kind of XML/HTML-matching benchmark ([Yes, I know this is a bad idea](http://stackoverflow.com/questions/1732348/regex-match-open-tags-except-xhtml-self-contained-tags)). If you know how to benchmark regexps, feel free to [fork away](http://github.com/michaeledgar/ruby-boost-regex/fork)!

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

You're going to need Boost.Regex.  You can find the [Boost Project here](http://www.boost.org/), with a
[Getting Started page](http://www.boost.org/doc/libs/1_42_0/more/getting_started/index.html) for installation.

Ruby-boost-regex expects you to have the boost regex library available as "libboost\_regex.so" or "libboost\_regex.dylib"
or whatever your platform specifies - the key point is the name "boost\_regex".

    gem install ruby-boost-regex
    
For the Unicode support I'm trying to get in, you'll need IBM's [ICU project](http://site.icu-project.org/) installed, 
with the recommended download page you start from [being ICU4C 4.4](http://icu-project.org/download/4.4.html#ICU4C). In addition,
your Boost libraries must be [configured to use ICU](http://www.boost.org/doc/libs/1_42_0/libs/regex/doc/html/boost_regex/install.html#boost_regex.install.building_with_unicode_and_icu_support).

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
