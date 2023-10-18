#include <iostream>
#include <sstream>
using namespace std;

namespace indent {
  class indentbuf: public streambuf {
  public:
    static const int idx;

    indentbuf(ostream& orig, size_t indent)
    : orig(orig),
      orig_buf(orig.rdbuf()),
      comment_on(false),
      indent(indent),
      wrap(70),
      lookback(30)
    {
      orig.rdbuf(this);
      update_prefix();
    }

    virtual ~indentbuf() {
      pubsync();
      orig.rdbuf(orig_buf);
    }

    void adjust_indent(size_t adj) {
      indent += adj;
      update_prefix();
    }

    void set_indent(size_t val) {
      indent = val;
      update_prefix();
    }

    void set_wrap(size_t wrap_val, size_t lookback_val) {
      wrap = wrap_val;
      lookback = lookback_val;
    }

    // Comment will only last until the next natural newline.
    void comment() {
      comment_on = true;
    }

  protected:
    int_type overflow(int_type ch) {
      // If we get an eof just return the right stuff.
      if (traits_type::eq_int_type(ch, traits_type::eof())) {
        return traits_type::not_eof(ch);
      }

      buffer += ch;
      if (ch == '\n' || ch == '\r') {
        buffer = prefix + buffer;
        // Ok.. we just got a new line, so output the buffer in wrapped chunks.
        while (buffer.size() > wrap) {
          // find the last space before the wrap point.
          size_t wrap_pos = buffer.find_last_of("\n ", wrap);
          size_t extra = 1;
          // If we didn't find a space, just wrap at the wrap point.
          if (wrap_pos == string::npos || (wrap - wrap_pos > lookback)) { wrap_pos = wrap; extra = 0; }
          if (comment_on) orig_buf->sputc(';');
          orig_buf->sputn(buffer.c_str(), wrap_pos);
          orig_buf->sputc('\n');
          buffer = buffer.substr(wrap_pos + extra);
          if (buffer.size() > 0) {
            buffer = prefix + string(2, ' ') + buffer;
          }
        }
        if (buffer.size() > 0) {
          if (comment_on) orig_buf->sputc(';');
          orig_buf->sputn(buffer.c_str(), buffer.size());
        }
        buffer.clear();
        // all output, turn off comment now.
        comment_on = false;
      }
      return ch;
    }

    int_type sync(void) {
      if (buffer.size() > 0) {
        if (comment_on) orig_buf->sputc(';');
        buffer = prefix + buffer;
        orig_buf->sputn(buffer.c_str(), buffer.size());
        buffer.clear();
      }
      return orig_buf->pubsync();
    }
  private:
    void update_prefix() {
      if (indent < 0) indent = 0;
      if (indent) prefix = string(indent, ' ');
      else prefix.clear();
    }

    ostream& orig;
    streambuf *orig_buf;
    bool comment_on;
    size_t indent;
    size_t wrap;
    size_t lookback;
    string buffer;
    string prefix;
  };

  ostream &wrap(ostream &os);
  ostream &comment(ostream &os);
  ostream &incr(ostream &os);
  ostream &decr(ostream &os);
  ostream &clear(ostream &os);

  struct wrap_data { size_t wrap; size_t lookback; };
  inline wrap_data wrap(size_t wrap=70, size_t lookback=30) {
    return { wrap, lookback };
  }
  template<typename _CharT, typename _Traits>
    inline basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& os, wrap_data data) {
    if(os.pword(indentbuf::idx) == nullptr) {
      indentbuf *newbuf = new indentbuf(os, 0);
      newbuf->set_wrap(data.wrap, data.lookback);
      os.pword(indentbuf::idx) = newbuf;
    } else {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      buf->set_wrap(data.wrap, data.lookback);
    }
    return os;
  }

  struct indent_data { size_t indent; };
  inline indent_data set_indent(size_t indent=2) {
    return { indent };
  }
  template<typename _CharT, typename _Traits>
    inline basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& os, indent_data data) {
    if(os.pword(indentbuf::idx) == nullptr) {
      indentbuf *newbuf = new indentbuf(os, data.indent);
      os.pword(indentbuf::idx) = newbuf;
    } else {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      buf->set_indent(data.indent);
    }
    return os;
  }
}
