module Boost
  class Regexp
    
    class << self
      def enable_monkey_patch!
        ::Regexp.__send__(:define_method, :boost!) do |*args|
          flags = args.any? ? args.first : Boost::Regexp::NORMAL
          Boost::Regexp.new(self.source, flags)
        end
      end
    end
    
    def casefold?
      (self.options & IGNORECASE) > 0
    end
    
    def inspect
      base = "/#{source}/"
      base << "m" if options & Boost::Regexp::DOTS_MATCH_NEWLINES > 0
      base << "i" if options & Boost::Regexp::IGNORECASE > 0
      base << "x" if options & Boost::Regexp::IGNORE_WHITESPACE > 0
      base
    end
  end
end