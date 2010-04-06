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
  end
end