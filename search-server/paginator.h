#pragma once

#include <ostream>
#include <vector>

#include "document.h"

using namespace std;

template <typename TDocument = Document>
class Page : protected vector<TDocument> {
   public:
    using InIterator = typename Page::const_iterator;

    Page() : vector<TDocument>() {}

    explicit Page(int count) : vector<TDocument>(count) {}

    Page(int count, const TDocument& doc) : vector<TDocument>(count, doc) {}

    Page(InIterator begin, InIterator end) : vector<TDocument>(begin, end) {}

    friend ostream& operator<<(ostream& os, Page page) {
        for (auto ptr = page.begin(); ptr != page.end(); ptr++) {
            os << *ptr;
        }
        return os;
    }

    void AddDocument(const TDocument& document);

    // Container interface
    void Resize(size_t new_size);
    void Reserve(size_t n);
    bool IsEmpty() const;
    size_t Size() const;
    void Clear();

    // Iterator interface
    auto begin() const;
    auto end() const;

   private:
    using super = vector<TDocument>;
};

template <class TPage = Page<Document>>
class Paginator : protected vector<TPage> {
   public:
    using DocIterator = typename TPage::InIterator;

    explicit Paginator(size_t page_size) : page_size_{page_size} {}
    Paginator(DocIterator begin, DocIterator end, size_t page_size);

    /// Add new page
    void AddPage(TPage page);

    /// Get page by index.
    /// This function provides for safer data access.
    TPage& GetPage(size_t index);

    /// Get page by index.
    /// This function provides for safer data access.
    const TPage& GetPage(size_t index) const;

    /// Get page by index
    TPage& operator[](size_t index) {
        return super::operator[](index);
    }

    /// First iteration element
    auto begin() const;

    /// Last iteration element
    auto end() const;

    /// Total page count
    size_t size() const;

    /// Get size of page
    size_t PageSize() const;

   private:
    using super = vector<TPage>;

    size_t page_size_;
};

// ----------------------------------------------------------------
// Paginator implementation
// ----------------------------------------------------------------

template <class TPage>
Paginator<TPage>::Paginator(DocIterator begin, DocIterator end, size_t page_size) : Paginator(page_size) {
    TPage curr_page{};
    curr_page.Reserve(page_size);
    int i = 0;
    for (auto ptr = begin; ptr != end; ptr++) {
        curr_page.AddDocument(*ptr);
        if (++i > 0 && i % page_size_ == 0) {
            this->AddPage(curr_page);
            curr_page.Clear();
        }
    }
    if (!curr_page.IsEmpty()) {
        curr_page.Resize(curr_page.Size());
        this->AddPage(curr_page);
    }
}

template <class TPage>
void Paginator<TPage>::AddPage(TPage page) {
    this->push_back(page);
}

template <class TPage>
TPage& Paginator<TPage>::GetPage(size_t index) {
    return super::at(index);
}

template <class TPage>
const TPage& Paginator<TPage>::GetPage(size_t index) const {
    return super::at(index);
}

template <class TPage>
auto Paginator<TPage>::begin() const {
    return super::begin();
}

template <class TPage>
auto Paginator<TPage>::end() const {
    return super::end();
}

template <class TPage>
size_t Paginator<TPage>::size() const {
    return super::size();
}

// ----------------------------------------------------------------
// Page implementation
// ----------------------------------------------------------------

template <typename TDocument>
void Page<TDocument>::AddDocument(const TDocument& document) {
    super::push_back(document);
}

template <typename TDocument>
void Page<TDocument>::Resize(size_t new_size) {
    super::resize(new_size);
}

template <typename TDocument>
void Page<TDocument>::Reserve(size_t n) {
    super::reserve(n);
}

template <typename TDocument>
bool Page<TDocument>::IsEmpty() const {
    return super::empty();
}

template <typename TDocument>
size_t Page<TDocument>::Size() const {
    return super::size();
}

template <typename TDocument>
void Page<TDocument>::Clear() {
    super::clear();
}

template <typename TDocument>
auto Page<TDocument>::begin() const {
    return super::begin();
}

template <typename TDocument>
auto Page<TDocument>::end() const {
    return super::end();
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
