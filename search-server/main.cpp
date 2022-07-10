#include <cassert>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>

using namespace std;

class Editor {
   public:
    using Iterator = std::list<char>::iterator;
    Editor() : cursor_{text_.begin()} {}
    // сдвинуть курсор влево
    void Left() {
        // assert(!text_.empty() && cursor_ != text_.begin());
        if (cursor_ == text_.begin()) {
            return;
        }
        --cursor_;
    }
    // сдвинуть курсор вправо
    void Right() {
        // assert(!text_.empty() && cursor_ != text_.end());
        if (cursor_ == text_.end()) {
            return;
        }
        ++cursor_;
    }
    // вставить символ token
    void Insert(char token) {
        cursor_ = text_.insert(cursor_, token);
        ++cursor_;
    }
    // вырезать не более tokens символов, начиная с текущей позиции курсора
    void Cut(size_t tokens = 1) {
        buffer_.clear();
        auto ptr = cursor_;
        for (; ptr != text_.end() && tokens > 0; --tokens) {
            buffer_.push_back(*ptr);
            ptr = text_.erase(ptr);
        }
        cursor_ = ptr;
    }
    void Copy(size_t tokens = 1) {
        buffer_.clear();
        for (auto ptr = cursor_; ptr != text_.end() && tokens > 0; --tokens, ++ptr) {
            buffer_.push_back(*ptr);
        }
    }
    // вставить содержимое буфера в текущую позицию курсора
    void Paste() {
        if (buffer_.empty()) {
            return;
        }
        auto dest_ptr = cursor_;
        for (auto src_ptr = buffer_.begin(); src_ptr != buffer_.end(); ++src_ptr) {
            dest_ptr = text_.insert(dest_ptr, *src_ptr);
            dest_ptr++;
        }
        // cursor_ = dest_ptr == text_.end() ? dest_ptr : next(dest_ptr);
        // cursor_ = dest_ptr != cursor_ ? ++dest_ptr : dest_ptr;
        cursor_ = dest_ptr;
    }
    // получить текущее содержимое текстового редактора
    string GetText() const {
        return string(text_.begin(), text_.end());
    }

   private:
    list<char> buffer_;
    list<char> text_;
    Iterator cursor_;
};

int main() {
    Editor editor;
    const string text = "hello, world"s;
    for (char c : text) {
        editor.Insert(c);
    }
    // Текущее состояние редактора: `hello, world|`
    for (size_t i = 0; i < text.size(); ++i) {
        editor.Left();
    }
    // Текущее состояние редактора: `|hello, world`
    editor.Cut(7);
    // Текущее состояние редактора: `|world`
    // в буфере обмена находится текст `hello, `
    for (size_t i = 0; i < 5; ++i) {
        editor.Right();
    }
    // Текущее состояние редактора: `world|`
    editor.Insert(',');
    editor.Insert(' ');
    // Текущее состояние редактора: `world, |`
    editor.Paste();
    // Текущее состояние редактора: `world, hello, |`
    editor.Left();
    editor.Left();
    //Текущее состояние редактора: `world, hello|, `
    editor.Cut(3);  // Будут вырезаны 2 символа
    // Текущее состояние редактора: `world, hello|`
    cout << editor.GetText();

    assert(editor.GetText() == "world, hello"s);
    return 0;
}
