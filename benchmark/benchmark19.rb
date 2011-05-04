#!/usr/bin/env ruby

# The Computer Language Shootout
# http://shootout.alioth.debian.org/
#
# contributed by jose fco. gonzalez
# Adapted for the Ruby Benchmark Suite.
require 'benchmark'
require 'rubygems'
require 'ruby-boost-regex'
require 'lorem'

fname = File.dirname(__FILE__) + "/fasta.input" 
seq = File.read(fname)
seq.gsub!(/>.*\n|\n/,"")

def fair_scan(string, regex)
  result = []
  idx = -1
  while idx
    match = regex.match(string[(idx + 1)..-1])
    break unless match
    idx += match.begin(0) + 1
    result << match.to_a
  end
  result
end


regexes = [
  /agggtaaa|tttaccct/i,
  /[cgt]gggtaaa|tttaccc[acg]/i,
  /a[act]ggtaaa|tttacc[agt]t/i,
  /ag[act]gtaaa|tttac[agt]ct/i,
  /agg[act]taaa|ttta[agt]cct/i,
  /aggg[acg]aaa|ttt[cgt]ccct/i,
  /agggt[cgt]aa|tt[acg]accct/i,
  /agggta[cgt]a|t[acg]taccct/i,
  /agggtaa[cgt]|[acg]ttaccct/i
  ]
  
boost_regexes = [
  Boost::Regexp.new('agggtaaa|tttaccct',         Boost::Regexp::IGNORECASE),
  Boost::Regexp.new('[cgt]gggtaaa|tttaccc[acg]', Boost::Regexp::IGNORECASE),
  Boost::Regexp.new('a[act]ggtaaa|tttacc[agt]t', Boost::Regexp::IGNORECASE),
  Boost::Regexp.new('ag[act]gtaaa|tttac[agt]ct', Boost::Regexp::IGNORECASE),
  Boost::Regexp.new('agg[act]taaa|ttta[agt]cct', Boost::Regexp::IGNORECASE),
  Boost::Regexp.new('aggg[acg]aaa|ttt[cgt]ccct', Boost::Regexp::IGNORECASE),
  Boost::Regexp.new('agggt[cgt]aa|tt[acg]accct', Boost::Regexp::IGNORECASE),
  Boost::Regexp.new('agggta[cgt]a|t[acg]taccct', Boost::Regexp::IGNORECASE),
  Boost::Regexp.new('agggtaa[cgt]|[acg]ttaccct', Boost::Regexp::IGNORECASE)
]

puts "DNA-Matching (Computer Language Shootout)"
puts "========================================="
Benchmark.bmbm do |x|
    x.report("Boost regex")  { 100.times { boost_regexes.each { |reg| fair_scan(seq, reg)}} }
    x.report("1.9 Ruby regex") { 100.times { regexes.each { |reg| fair_scan(seq, reg)}} }
end


reg = /\d{3}-\d{3}-\d{4}/
boost_reg = Boost::Regexp.new('\d{3}-\d{3}-\d{4}')
text = Lorem::Base.new('paragraphs', 2000).output

puts ""
puts "Failing to match a phone number in a big string of text"
puts "======================================================="
Benchmark.bmbm do |x|
    x.report("1.9 Ruby regex") { 10000.times { fair_scan(text, reg)}}
    x.report("Boost regex")    { 10000.times { fair_scan(text, boost_reg)}}
end