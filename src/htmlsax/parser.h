#ifndef HTMLSAX_PARSER_H__
#define HTMLSAX_PARSER_H__

#include <cctype>
#include <string>
#include <vector>
#include <ostream>
#include <iterator>

#include <boost/algorithm/string/case_conv.hpp>

#ifdef _DEBUG
    #undef assert
    #include <windows.h>
    #define assert(x) do {if (!(x)) DebugBreak(); } while(0);
#endif

template<typename T>
class Range {
public:
  typedef T* iterator;
  typedef const T* const_iterator;

  const T* begin() const { return begin_; }
  const T* end() const { return end_; }
  size_t size() const { return std::distance(begin_, end_); }
  bool empty() const { return (begin_ == end_); }

  // why push_back:
  // 操作和 std::string 比较类似
  void push_back(char) { assert(begin_); end_ ++; }
  void clear() { begin_ = 0; end_ = 0; }
  void begin(const T* b) { begin_ = b; }
  void end(const T* e) { end_ = e; }

  explicit Range(const T* b = 0, const T* e = 0) : begin_(b), end_(e) {}
private:
  const T* begin_;
  const T* end_;
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const Range<T> & r) {
  stream.write(r.begin(), r.size());
  return stream;
}

#if 1
  typedef Range<char> crange;
#else
  typedef std::string crange;
#endif

typedef std::vector<std::pair<crange, crange> > attr_array;

enum TagState {
  // <tag attr="value">data</tag>
  //                  [ 进入条件 | 退出条件 ]
  // main state
  state_start = 1, // [ 文档开始 ]
  state_tag_start, // [ < |  ]
  state_tag_end,   // [ > |  ]
  state_data,      // [ after state_tag_end or state_start | first < ]
  state_end,       // [ 文档结束 ]

  // sub procedure state -- 注意容错
  //                    [ 进入state条件 | 离开state的条件 ]
  state_tag_name,    // [first not space | first space, >]
  state_attr_name,   // [ leave state_tag_name | space, = ]
  state_attr_split,  // [ leave state_attr_name | >, ", ', 根据是否有quote的情况决定 ]
  state_attr_value,  // [ leave state_attr_split | >, quote ]
  state_end_tag_symbol_pre,   // [ </ ] </tag> or </ tag> or <tag />
  state_end_tag_symbol_post,  // [ leave state_atte_name ] <tag />

  // TODO: delete
  state_quote,
  state_comment,  // <!-- * -->
  state_script,   // <script xxx> * </script>
  state_entities, // <![CDATA[ * ]]>
};

template<typename HandlerT>
class Parser {
public:
  Parser(HandlerT* handler) 
    : contine_(true), handler_(handler)
    , individual_script_(true)
  {}

  // template<class ForwardRange>
  bool Feed(const char* beg, const char* end, HandlerT* handler = 0);
  bool Feed(std::istream& stream, HandlerT* handler = 0) {
    //        std::getline(stream, buf_, '\0');
    buf_.assign(std::istreambuf_iterator<char>(stream.rdbuf()), 
      std::istreambuf_iterator<char>());
    bool f = Feed(buf_.c_str(), buf_.c_str() + buf_.size(), handler);
    buf_.clear();
    return f;
  }

  // options:
  // 是否吃掉 > 后，或者 < 前的space
  bool eat_space() const;
  void eat_space(bool f);

  static bool is_html_tag_char(int ch) {
    // 0 - 9 a - z A - Z /  //
    return (ch >= 'a' && ch <= 'z')
      || (ch >= 'A' && ch <= 'Z')
      || (ch >= '0' && ch <= '9')
      || (ch == '/') 
      || (ch == '!');
  }
private:
  enum TagChar {
    char_left_bracket       = '<',
    char_right_bracket      = '>',
    char_excalmatory_mark   = '!',
    char_space              = ' ',
    char_quote = '\'',
    char_dquote = '"',
    char_equal  = '=',
    char_escape = '\\',

    char_end_tag = '/',
    char_hyphen = '-',
    char_cdata_end = ']',
  };

  // 简单分隔 <tag>data</tag>
  TagState token_outof_tag(const char** ppbeg, const char* end);
  TagState token_in_tag(const char** ppbeg, const char* end);

  // 在 token_in_tag 中细分的 tag
  TagState token_entities(const char** ppbeg, const char* end);
  TagState token_script(const char** ppbeg, const char* end);

  // TODO: 细分 <tag attr=value> or </tag> 
  // 现在全部在 token_in_tag 完成

  // 
  TagState token_next(const char* beg, const char* end, TagState state_before, 
    TagState state_new);

  // 分析中保存指针
  TagState state_;
  crange curtag_name_;
  crange curattr_name_;
  crange curattr_value_;
  attr_array curattr_array_;
  crange curdata_;
  bool contine_;
  const char* curpos_;
  std::string buf_;

  // 所有调用 call_xxx 函后，再调用 post_call 把数据清空，以备下次使用
  void call_start_document();
  void call_start_element(crange& name, attr_array&);
  void call_characters(crange& text);
  void call_end_element(crange& name);
  void call_script(crange& text, attr_array&);
  void call_entities(crange& text);
  void call_end_document();

  void post_call_element();
  void post_call_document();
  void post_call_characters();
private:
  HandlerT* handler_;

  // script 调用 call_script
  unsigned long individual_script_ : 1;

  // noncopyable
  Parser(const Parser&);
  Parser& operator=(const Parser&);
};

template<typename HandlerT>
inline void Parser<HandlerT>::post_call_element() {
    curtag_name_.clear();
    curattr_array_.clear();
    curattr_name_.clear();
    curattr_value_.clear();
}

template<typename HandlerT>
inline void Parser<HandlerT>::post_call_document() {
    buf_.clear();
}

template<typename HandlerT>
inline void Parser<HandlerT>::post_call_characters() {
    curdata_.clear();
}


template<typename HandlerT> 
inline bool Parser<HandlerT>::Feed(const char* beg, const char* end, 
                                      HandlerT* handler) {
  if (handler) handler_ = handler;
  if (!handler_) return false;

  state_ = state_start;
  call_start_document();

#ifdef _DEBUG
  const char * pre_pos = 0;
#endif

  const char* p = beg;
  while (p != end && contine_)
  {
    const char* current_pos = p;

    TagState state_new = state_;

    switch(state_)
    {
    case state_start:
      state_new = token_outof_tag(&current_pos, end);
      break;
    case state_tag_start:
      assert(*current_pos == '<');

      state_new = token_in_tag(&current_pos, end);

      if (current_pos != end)
        assert(*(current_pos-1) == '>');

      break;
    case state_tag_end:  // fall
    case state_data:
      state_new = token_outof_tag(&current_pos, end);
      break;
    case state_end:
      break;
    default:
      assert(false);
      break;
    }

#ifdef _DEBUG
    assert(current_pos > pre_pos || state_ != state_new);
    assert(current_pos <= end);
    pre_pos = current_pos;
#endif

    state_ = state_new;
    p = current_pos;
    // 如果有 state_new 返回值不确定的 state, 才需要在 token_next
    // 中纠正
    // state_ = token_next(p, end, state_, state_new);
  }

  state_ = state_end;
  call_end_document();
  post_call_document();

  return true;
}

// enter state_data                     | leave state_data
//  after state_tag_end or state_start  | first < (出现 < !! 出现合法的 <tag_char)

template<typename HandlerT>
TagState Parser<HandlerT>::token_outof_tag(const char** ppbeg, const char* end) {
  bool all_space = true;
  TagState es = state_end;
  const char* i = *ppbeg;
  while (i != end)
  {
    // space
    // <
    // first not space

    if (*i == char_left_bracket)
    {
      if (i + 1 != end && is_html_tag_char(*(i + 1)))
      {
        es = state_tag_start;
        break;
      }
    }

    // TODO: 加上配置项，决定是否过滤空格
    if (all_space && *i != char_space)
    {
      all_space = false;
      curdata_.begin(i);
    }

    ++ i;
  }

  if (curdata_.begin() && !curdata_.end())
  {
    if (state_end == es)
      assert(i == end);
    curdata_.end(i);
  }

  if (curdata_.begin() && curdata_.end() && !curdata_.empty())
  {
    call_characters(curdata_);
    post_call_characters();
  }

  *ppbeg = i;
  return es;
}

// state_tag_start |  state_tag_end
// < |             |   > (出现 > )
template<typename HandlerT>
TagState Parser<HandlerT>::token_in_tag(const char** ppbeg, const char* end)
{
  assert(*ppbeg && **ppbeg == char_left_bracket);

  const char* i = *ppbeg + 1;
  TagState es = state_tag_start;

  int cur_quote = -1;

  // 当遇到space时都假定后面全部是space，代码保持风格一致
  // if (all_space && found a not space) 
  // {
  //      all_space = false;
  //      found sth. 有意义
  //  }
  // 
  // if (!all_space && found a space)
  //      all_space = true;

  bool all_space = true;
  bool escaped = false; // 属性字符串中含有 \ //
  bool exist_equal = false; // 是否出现过 =

  const unsigned int tag_start = 0x1;
  const unsigned int tag_end = 0x2;
  // <tag> or </tag> or <tag />
  //   1        2         1|2 = 3
  unsigned int tag_type = tag_start; // default value must be tag_start

  // < 后面的空格都认为是非法，导致 < /span> or < span> 都解析成
  // characters data
  // 

  while (i != end && es != state_tag_end && es != state_end)
  {
    // <
    // !
    // space
    // /
    // >
    // '"

    switch(es)
    {
    case state_tag_start:
      // 回溯到 state_data 状态
      if (!is_html_tag_char(*i))
      {
        es = state_data;
        return es;
      }
      else
      {
        // </ end>
        if (*i == char_end_tag)
        {
          tag_type = tag_end;
          all_space = true;
          es = state_end_tag_symbol_pre;
        }
        else if (*i == char_excalmatory_mark)
        {
          es = token_entities(&i, end);
          assert(es == state_tag_end);
        }
        else
        {
          es = state_tag_name;
          curtag_name_.begin(i);
        }
      }
      break;
    case state_end_tag_symbol_pre:
      if (all_space && *i > char_space)
      {
        assert(tag_end == tag_type);
        es = state_tag_name;
        curtag_name_.begin(i);
      }
      break;
    case state_tag_name:
      if (*i == char_right_bracket)
      {
        es = state_tag_end;
        curtag_name_.end(i);
      }
      else if (*i == char_space)
      {
        es = state_attr_name;
        curtag_name_.end(i);
        all_space = true;
      }
      else if (*i == char_end_tag)
      {
        es = state_end_tag_symbol_post;
        curtag_name_.end(i);
        tag_type |= tag_end;
      }
      break;
    case state_end_tag_symbol_post:
      if (*i == char_right_bracket)
      {
        es = state_tag_end;
        tag_type |= tag_end;
      }
      else
        all_space = false;
      break;
    case state_attr_name:
      if (all_space && *i > char_space)
      {
        // <tag />
        if (*i == char_end_tag)
        {
          es = state_end_tag_symbol_post;                    
          break;
        }

        curattr_name_.begin(i);
        all_space = false;
      }

      if (!all_space)
      {
        // 1 tag name end
        // 2 attr_name begin
        if (*i == char_space || *i == char_equal)
        {
          es = state_attr_split;

          curattr_name_.end(i);

          all_space = true;
          exist_equal = false;
          continue;
        }
        else if (*i == char_right_bracket)
        {
          es = state_tag_end;
          curattr_name_.end(i);
        }
      }
      break;
    case state_attr_split:
      // must be: space =
      // end with: ' " / text
      // TODO: *i > char_space 合适吗？
      //            if (!exist_equal)
      //                assert(*i <= char_space || *i == char_equal);

      if (*i > char_space)
      {
        // 是否有 = 很重要 eg: <a title href="xxx">
        if (*i == char_equal)
          exist_equal = true;
        else
        {
          if (*i == char_dquote || *i == char_quote)
          {
            cur_quote = *i;

            es = state_attr_value;
          }
          else
          {
            cur_quote = -1;

            es = exist_equal ? state_attr_value : state_attr_name;
            continue;
          }
        }





        // TODO: =" 后面出现的 space 都是有效的
        // 在 state_attr_value 处理有错误 :(
        // all_space = true;

        //if (-1 == cur_quote)
        //    continue;
      }
      break;
    case state_attr_value:
      if (!curattr_value_.begin())
      {
        curattr_value_.begin(i);
      }

      {
        // 这儿似乎有bug
        // TODO: 测试多次转义的html

        if (!escaped && *i == char_escape)
          escaped = true;

        // end with: cur_quote / > / space
        if ( (cur_quote == -1 && (*i <= char_space || *i == char_right_bracket))
          || (cur_quote == *i && !escaped) )
        {
          curattr_value_.end(i);
          curattr_array_.push_back(
            std::make_pair(curattr_name_, curattr_value_)
            );

          curattr_value_.clear();
          curattr_value_.clear();

          if (*i == char_right_bracket)
            es = state_tag_end;
          else
            es = state_attr_name;

          all_space = true;
        }

        if (escaped && *i != char_escape)
          escaped = false;
      }
      break;
    case state_tag_end: // fall to default
    default:
      assert(false);
      break;
    }

    // ???
    if (i != end && es != state_end)
      ++ i;
  }

  if (!curtag_name_.empty() && curtag_name_.end())
  {
    assert(curtag_name_.end());

    bool script = false;
    // script special
    if (*curtag_name_.begin() == 's' || *curtag_name_.begin() == 'S')
    {
      std::string tagname(curtag_name_.begin(), curtag_name_.end());
      boost::to_lower(tagname);

      if (tagname == "script")
      {
        script = true;
        if (tag_type & tag_start)
        {
          es = token_script(&i, end);
          assert(state_tag_end == es);
        }
      }
    }

    if (!script)
    {
      if (tag_type & tag_start)
        call_start_element(curtag_name_, curattr_array_);

      if (tag_type & tag_end)
        call_end_element(curtag_name_);
    }

    post_call_element();
  }

  *ppbeg = i;

  return es;
}

// enter: < 后出现 !
// leave: --> or 
template<typename HandlerT>
TagState Parser<HandlerT>::token_entities(const char** ppbeg, const char* end) {
  assert(**ppbeg == char_excalmatory_mark);

  TagState es = state_end;
  crange text;
  const char* p = *ppbeg;

  // <!DOCTYPE >
  // <![CDATA[ ]]>
  // <!-- -->

  // ugly resolution
  text.begin(*ppbeg);
  int hyphen_count = 0;
  bool only_end_tag = true;

  if (*(p+1) == char_hyphen || *p == '[')
    only_end_tag = false;

  while (p != end)
  {
    if (only_end_tag)
    {
      if (*p == char_right_bracket)
      {
        *ppbeg = p;
        return state_tag_end;
      }
    }
    else
    {
      if (*p == char_hyphen || *p == char_cdata_end)
      {
        ++ hyphen_count;
      }
      else
      {
        if (*p == char_right_bracket)
        {
          if (2 <= hyphen_count)
          {
            *ppbeg = p;
            return state_tag_end;
          }
        }
        else
        {
          hyphen_count = 0;
        }
      }
    }

    ++p;
  }

  // TODO:
  // call_entities(text);

  *ppbeg = p;
  return es;
}

// *ppbeg = text</script>
// <script xxx> 已经解析过了，没有必要在这儿再解析
template<typename HandlerT>
TagState Parser<HandlerT>::token_script(const char** ppbeg, const char* end) {
  assert(*ppbeg && *(*ppbeg-1) == char_right_bracket);

  TagState es = state_start;
  crange text;
  const char* i = *ppbeg;

  text.begin(i);

  // text </           script >
  // text left_bracket script right_bracket
  const int local_state_text = 1;
  const int local_state_left_bracket = 2;
  const int local_state_script = 3;

  int local_state_ = local_state_text;
  const char* text_end = 0;
  const char* script_begin = 0;
  // const int script_length = 6; // script

  int cur_quote = -1;
  bool escaped = false;

  while (i != end && state_start == es)
  {
    switch (local_state_)
    {
    case local_state_text:
      if (cur_quote == -1)
      {
        if (*i == char_dquote || *i == char_quote)
          cur_quote = *i;

        else if (*i == char_left_bracket)
        {
          local_state_ = local_state_left_bracket;
          text_end = i;
        }
      }
      else // (cur_quote != -1)
      {
        if (!escaped && *i == char_escape)
          escaped = true;

        if (!escaped && *i != char_escape)
        {
          if (!escaped && cur_quote == *i)
            cur_quote = -1;
        }

        if (escaped && *i != char_escape)
          escaped = false;
      }
      break;
    case local_state_left_bracket:
      assert(text_end);
      assert(cur_quote == -1);
      if (*i == char_end_tag)  // '/'
      {
        local_state_ = local_state_script;
      }
      else
      {
        local_state_ = local_state_text;
        text_end = 0;
        continue;
      }
      break;
    case local_state_script:
      assert(text_end);
      assert(cur_quote == -1);
      if (0 == script_begin)
        script_begin = i;
      else if(*i == char_right_bracket)
      {
        assert(script_begin);
        assert(i > script_begin);

        bool reverse = true;
        if (std::distance(script_begin, i) >= 6 
          && (*script_begin=='s' || *script_begin == 'S'))
        {
          std::string script_text(script_begin, script_begin + 6);
          boost::to_upper(script_text);

          if (script_text == "SCRIPT")
          {
            assert(text_end);
            reverse = false;
            text.end(text_end);
            es = state_tag_end;
          }
        }

        if (reverse)
        {
          local_state_ = local_state_text;
          text_end = 0;
          script_begin = 0;
        }
      }
      break;
    }

    ++i;
  }

  assert(text.end() && text.begin());
  assert(*(i-1) == '>');

  text.end(text_end);
  call_script(text, curattr_array_);

  *ppbeg = i;
  return es;
}

// 纠正可能出现的 state 错误
template<typename HandlerT>
TagState Parser<HandlerT>::token_next(const char* beg, const char* end, 
                                          TagState state_before, TagState state_new) {
  if (state_tag_end == state_new) {
    if (*beg == char_left_bracket)
      return state_tag_start;
    else
      return state_data;
  }
  return state_new;
}

template<typename HandlerT>
void Parser<HandlerT>::call_start_document() {
  contine_ = handler_->start_document();
}

template<typename HandlerT>
void Parser<HandlerT>::call_start_element(crange& name, attr_array& attrlist) {
  assert( name.begin() && name.end() );
  assert( name.begin() < name.end() );

  contine_ = handler_->start_element(name, attrlist);
}

template<typename HandlerT>
void Parser<HandlerT>::call_characters(crange& text) {
  assert( text.begin() && text.end() );
  assert( text.begin() < text.end() );

  contine_ = handler_->characters(text);
}

template<typename HandlerT>
void Parser<HandlerT>::call_end_element(crange& name) {
  assert( name.begin() && name.end() );
  assert( name.begin() < name.end() );

  contine_ = handler_->end_element(name);
}

template<typename HandlerT>
void Parser<HandlerT>::call_script(crange& text, attr_array& attrlist) {
  assert( text.begin() && text.end() );
  assert( text.begin() <= text.end() );

  contine_ = handler_->script(text, attrlist);
}

template<typename HandlerT>
void Parser<HandlerT>::call_entities(crange& text) {
  assert( text.begin() && text.end() );
  assert( text.begin() < text.end() );

  contine_ = handler_->entities(text);
}

template<typename HandlerT>
void Parser<HandlerT>::call_end_document() {
  contine_ = handler_->end_document();

  assert(state_ == state_end);
  assert(curattr_array_.size() == 0);
}

//////////////////////////////////////////////////////////////////////////

class DummyHandler {
public:
  bool start_document() {
    return true;
  }
  bool start_element(const crange& name, const attr_array&) {
    return true;
  }
  bool characters(const crange& text) {
    return true;
  }
  bool end_element(const crange& name) {
    return true;
  }
  bool entities(const crange& text) {
    return true;
  }
  bool script(const crange& text, const attr_array&) {
    return true;
  }
  bool end_document() {
    return true;
  }
};


#endif // HTMLSAX_PARSER_H__
