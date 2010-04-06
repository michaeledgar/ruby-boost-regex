module Boost
  class Regexp
    def casefold?
      (self.options & IGNORECASE) > 0
    end
  end
end