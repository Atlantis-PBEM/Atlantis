#include "indenter.hpp"

namespace indent {
  const int indentbuf::idx = std::ios_base::xalloc();

  std::ostream& wrap(std::ostream &os) {
    os << wrap(70);
    return os;
  }

  std::ostream& comment(std::ostream &os) {
    if(os.pword(indentbuf::idx) != nullptr) {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      buf->comment();
    } else {
      indentbuf *newbuf = new indentbuf(os, 0);
      newbuf->comment();
      os.pword(indentbuf::idx) = newbuf;
    }
    return os;
  }

  std::ostream& incr(std::ostream& os) {
    if(os.pword(indentbuf::idx) != nullptr) {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      buf->adjust_indent(2);
    } else {
      indentbuf *newbuf = new indentbuf(os, 2);
      os.pword(indentbuf::idx) = newbuf;
    }
    return os;
  }

  std::ostream& decr(std::ostream& os) {
    if(os.pword(indentbuf::idx) != nullptr) {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      buf->adjust_indent(-2);
    }
    return os;
  }

  // This removes all indenting, wrapping and commenting from the output stream.
  std::ostream& clear(std::ostream& os) {
    if(os.pword(indentbuf::idx) != nullptr) {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      delete buf;
      os.pword(indentbuf::idx) = nullptr;
    }
    return os;
  }
}
