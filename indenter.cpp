#include "indenter.hpp"

namespace indent {
  const int indentbuf::idx = ios_base::xalloc();

  ostream &wrap(ostream &os) {
    os << wrap(70);
    return os;
  }

  ostream &comment(ostream &os) {
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

  ostream& incr(ostream& os) {
    if(os.pword(indentbuf::idx) != nullptr) {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      buf->adjust_indent(2);
    } else {
      indentbuf *newbuf = new indentbuf(os, 2);
      os.pword(indentbuf::idx) = newbuf;
    }
    return os;
  }

  ostream& decr(ostream& os) {
    if(os.pword(indentbuf::idx) != nullptr) {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      buf->adjust_indent(-2);
    }
    return os;
  }

  // This removes all indenting, wrapping and commenting from the output stream.
  ostream& clear(ostream& os) {
    if(os.pword(indentbuf::idx) != nullptr) {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      delete buf;
      os.pword(indentbuf::idx) = nullptr;
    }
    return os;
  }    
}
