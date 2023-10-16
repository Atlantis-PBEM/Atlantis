#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

namespace indent {
  class indentbuf: public streambuf {
  public:
    static const int idx;

    indentbuf(ostream& orig, size_t indent)
    : orig(orig),
      orig_buf(orig.rdbuf()),
      comment_on(false),
      suppress(false),
      line_indent(indent),
      wrap(70),
      lookback(30),
      string_wrap_indent(2)
    {
      orig.rdbuf(this);
      update_prefix();
    }

    virtual ~indentbuf() {
      pubsync();
      orig.rdbuf(orig_buf);
    }

    // This one needs to be int_type so that it can be + or - to the current indent.
    void adjust_indent(int_type adj) {
      pubsync();
      line_indent += adj;
      update_prefix();
    }

    void set_indent(size_t val) {
      pubsync();
      line_indent = val;
      update_prefix();
    }

    void push_indent(size_t val) {
      pubsync();
      line_indent_stack.push_back(line_indent);
      line_indent = val;
      update_prefix();
    }

    void pop_indent() {
      pubsync();
      if (line_indent_stack.size() == 0) return;
      line_indent = line_indent_stack.back();
      line_indent_stack.pop_back();
      update_prefix();
    }

    void set_wrap(size_t wrap_val, size_t lookback_val, size_t string_wrap_indent_val=2) {
      pubsync();
      wrap = wrap_val;
      lookback = lookback_val;
      string_wrap_indent = string_wrap_indent_val;
    }

    void set_suppress(bool suppress_val) {
      pubsync();
      suppress = suppress_val;
    }

    // Comment will only last until the next natural newline.
    void comment() {
      pubsync();
      comment_on = true;
    }

  protected:
    int_type overflow(int_type ch) {
      // If we get an eof just return the right stuff.
      if (traits_type::eq_int_type(ch, traits_type::eof())) {
        return traits_type::not_eof(ch);
      }
      
      if (suppress) {
        return orig_buf->sputc(ch);
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
          // If the remaining buffer is just a newline, clear it
          if (buffer == "\n") buffer.clear();
          if (buffer.size() > 0) {
            buffer = prefix + string(string_wrap_indent, ' ') + buffer;
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
      if (line_indent < 0) line_indent = 0;
      // Just make sure we never get a huge indent.
      if (line_indent > 30) line_indent = 30;
      if (line_indent) prefix = string(line_indent, ' ');
      else prefix.clear();
    }

    ostream& orig;
    streambuf *orig_buf;
    bool comment_on;
    bool suppress;
    int_type line_indent;
    size_t wrap;
    size_t lookback;
    size_t string_wrap_indent;
    string buffer;
    string prefix;
    vector<int_type> line_indent_stack;
  };

  ostream &wrap(ostream &os);
  ostream &comment(ostream &os);
  ostream &incr(ostream &os);
  ostream &decr(ostream &os);
  ostream &clear(ostream &os);

  struct wrap_data { size_t wrap; size_t lookback; size_t string_wrap_indent; };
  inline wrap_data wrap(size_t wrap=70, size_t lookback=30, size_t string_wrap_indent=2) {
    return { wrap, lookback, string_wrap_indent };
  }
  template<typename _CharT, typename _Traits>
  inline basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& os, wrap_data data) {
    if(os.pword(indentbuf::idx) == nullptr) {
      indentbuf *newbuf = new indentbuf(os, 0);
      newbuf->set_wrap(data.wrap, data.lookback, data.string_wrap_indent);
      os.pword(indentbuf::idx) = newbuf;
    } else {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      buf->set_wrap(data.wrap, data.lookback, data.string_wrap_indent);
    }
    return os;
  }

  struct indent_data { size_t indent; };
  inline indent_data set_indent(size_t indent=2) { return { indent }; }
  template<typename _CharT, typename _Traits>
  inline basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& os, indent_data data) {
    if(os.pword(indentbuf::idx) == nullptr) {
      indentbuf *newbuf = new indentbuf(os, data.indent);
      os.pword(indentbuf::idx) = newbuf;
    } else {
      indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
      buf->set_indent(data.indent);
    }
    return os;
  }

  struct suppress_data { bool suppress; };
  inline suppress_data suppress_format(bool suppress) { return { suppress }; }
  template<typename _CharT, typename _Traits>
  inline basic_ostream<_CharT, _Traits>&operator<<(basic_ostream<_CharT, _Traits>& os, suppress_data data) {
    if(os.pword(indentbuf::idx) == nullptr) {
      indentbuf *newbuf = new indentbuf(os, 0);
      os.pword(indentbuf::idx) = newbuf;
    }
    indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
    buf->set_suppress(data.suppress);
    return os;
  }

  struct push_indent_data { size_t indent; bool push; };
  inline push_indent_data push_indent(size_t indent) { return { indent, true }; }
  inline push_indent_data pop_indent() { return { 0, false }; }
  template<typename _CharT, typename _Traits>
  inline basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& os, push_indent_data data) {
    if(os.pword(indentbuf::idx) == nullptr) {
      indentbuf *newbuf = new indentbuf(os, 0);
      os.pword(indentbuf::idx) = newbuf;
    }
    indentbuf *buf = static_cast<indentbuf *>(os.pword(indentbuf::idx));
    (data.push ? buf->push_indent(data.indent) : buf->pop_indent());
    return os;
  }
}
