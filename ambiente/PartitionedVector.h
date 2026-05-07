#ifndef _PARTITIONED_VECTOR_H
#define _PARTITIONED_VECTOR_H

#include <vector>
#include <algorithm>
#include <cassert>
#include <utility>

// =====================================================
// PartitionedVector
//
// Logical layout:
//
// [ ACTIVE | FIXED | PRICED_OUT ]
//   begin     A_end    F_end      end
//
// Invariants:
// - begin <= _active_end <= _fixed_end <= end
// - No erase: all transitions are O(1) via swaps
//
// =====================================================

template <class T, class Policy>
class PartitionedVector {
public:
    using Container = std::vector<T>;
    using Iterator = typename Container::iterator;
    using ConstIterator = typename Container::const_iterator;
    using ValueType = typename Container::value_type;
    using SizeType = typename Container::size_type;
    using Reference = typename Container::reference;
    using ConstReference = typename Container::const_reference;
    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using value_type = ValueType;
    using size_type = SizeType;
    using reference = Reference;
    using const_reference = ConstReference;

    // =====================================================
    // Data
    // =====================================================
    Container _data;

    Iterator _active_end; // end of ACTIVE region
    Iterator _fixed_end;  // end of FIXED region (start of PRICED_OUT)

    Policy _policy;

public:

    PartitionedVector() {
        InitEmpty();
    }

    // =====================================================
    // Initialization
    // =====================================================

    void InitEmpty() {
        _active_end = _data.begin();
        _fixed_end = _data.begin();
    }

    void InitAllActive() {
        _active_end = _data.end();
        _fixed_end = _data.end();
    }

    // =====================================================
    // Access (compatibility layer)
    // =====================================================

    Iterator begin() { return _data.begin(); }
    Iterator end() { return _data.end(); }
    ConstIterator begin() const { return _data.begin(); }
    ConstIterator end() const { return _data.end(); }
    ConstIterator cbegin() const { return _data.cbegin(); }
    ConstIterator cend() const { return _data.cend(); }

    Reference operator[](SizeType pos) { return _data[pos]; }
    ConstReference operator[](SizeType pos) const { return _data[pos]; }

    Reference at(SizeType pos) { return _data.at(pos); }
    ConstReference at(SizeType pos) const { return _data.at(pos); }

    Reference front() { return _data.front(); }
    ConstReference front() const { return _data.front(); }

    Reference back() { return _data.back(); }
    ConstReference back() const { return _data.back(); }

    bool empty() const { return _data.empty(); }
    SizeType size() const { return _data.size(); }
    SizeType capacity() const { return _data.capacity(); }

    void reserve(SizeType new_cap) {
        PreservePartitionIterators([&]() {
            _data.reserve(new_cap);
        });
    }

    void shrink_to_fit() {
        PreservePartitionIterators([&]() {
            _data.shrink_to_fit();
        });
    }

    void push_back(ConstReference value) {
        PreservePartitionIterators([&]() {
            _data.push_back(value);
        });
    }

    void push_back(ValueType&& value) {
        PreservePartitionIterators([&]() {
            _data.push_back(std::move(value));
        });
    }

    template <class... Args>
    Reference emplace_back(Args&&... args) {
        PreservePartitionIterators([&]() {
            _data.emplace_back(std::forward<Args>(args)...);
        });
        return _data.back();
    }

    Iterator erase(Iterator pos) {
        auto active_offset = ActiveOffset();
        auto fixed_offset = FixedOffset();
        auto erase_offset = static_cast<SizeType>(pos - _data.begin());

        Iterator next = _data.erase(pos);

        if (erase_offset < active_offset)
            --active_offset;
        if (erase_offset < fixed_offset)
            --fixed_offset;

        RestorePartitionIterators(active_offset, fixed_offset);
        return _data.begin() + (next - _data.begin());
    }

    Iterator erase(Iterator first, Iterator last) {
        auto active_offset = ActiveOffset();
        auto fixed_offset = FixedOffset();
        auto first_offset = static_cast<SizeType>(first - _data.begin());
        auto last_offset = static_cast<SizeType>(last - _data.begin());
        auto erased_count = last_offset - first_offset;

        Iterator next = _data.erase(first, last);

        active_offset -= ErasedBeforeBoundary(first_offset, last_offset, erased_count, active_offset);
        fixed_offset -= ErasedBeforeBoundary(first_offset, last_offset, erased_count, fixed_offset);

        RestorePartitionIterators(active_offset, fixed_offset);
        return _data.begin() + (next - _data.begin());
    }

    void clear() {
        _data.clear();
        InitEmpty();
    }

    // =====================================================
    // Counts
    // =====================================================

    int ActiveCount() const {
        return static_cast<int>(_active_end - _data.begin());
    }

    int FixedCount() const {
        return static_cast<int>(_fixed_end - _active_end);
    }

    int PricedOutCount() const {
        return static_cast<int>(_data.end() - _fixed_end);
    }

    // =====================================================
    // Ranges
    // =====================================================

    std::pair<Iterator, Iterator> ActiveRange() {
        return { _data.begin(), _active_end };
    }

    std::pair<Iterator, Iterator> FixedRange() {
        return { _active_end, _fixed_end };
    }

    std::pair<Iterator, Iterator> PricedOutRange() {
        return { _fixed_end, _data.end() };
    }

    // =====================================================
    // SINGLE ELEMENT TRANSITIONS (O(1))
    // =====================================================

    void MoveActiveToFixed(Iterator it) {
        _policy.OnMoveToFixed(*it);
        _active_end--;
        std::iter_swap(it, _active_end);
    }

    void MoveFixedToActive(Iterator it) {
        _policy.OnMoveToActive(*it);
        std::iter_swap(it, _active_end);
        _active_end++;
    }

    void MoveActiveToPricedOut(Iterator it) {
        _policy.OnMoveToPricedOut(*it);

        // Step 1: move to end of ACTIVE
        _active_end--;
        std::iter_swap(it, _active_end);

        // Step 2: move to end of FIXED (i.e., into PRICED_OUT)
        _fixed_end--;
        std::iter_swap(_active_end, _fixed_end);
    }

    void MovePricedOutToActive(Iterator it) {
        _policy.OnMoveToActive(*it);

        // Step 1: bring element to FIXED boundary
        std::iter_swap(it, _fixed_end);
        _fixed_end++;

        // Step 2: bring element to ACTIVE
        std::iter_swap(_fixed_end - 1, _active_end);
        _active_end++;
    }


    void MoveFixedToPricedOut(Iterator it) {
        _policy.OnMoveToPricedOut(*it);
        _fixed_end--;
        std::iter_swap(it, _fixed_end);
    }

    void MovePricedOutToFixed(Iterator it) {
        _policy.OnMoveToFixed(*it);
        std::iter_swap(it, _fixed_end);
        _fixed_end++;
    }

    // =====================================================
    // BATCH: PRICED_OUT → ACTIVE
    // (equivalent to CommitPriceIn)
    // =====================================================

    template <class Predicate>
    size_t CommitPriceIn(Predicate pred) {

        auto first = _fixed_end;
        auto last = _data.end();

		auto selected_end = std::stable_partition(first, last, pred);

        std::rotate(_active_end, first, selected_end);

        size_t count = std::distance(first, selected_end);

        _active_end += count;
        _fixed_end += count;

        for (auto it = _active_end - count; it != _active_end; ++it) {
            _policy.OnMoveToActive(*it);
            _policy.OnCommit(*it);
        }

        return count;
    }

    // =====================================================
    // BATCH: ACTIVE → PRICED_OUT
    // (equivalent to CommitPriceOut)
    // =====================================================

    template <class Predicate>
    size_t CommitPriceOut(Predicate pred) {

        auto begin_it = _data.begin();
        auto end_it = _active_end;

        auto remaining_end = std::stable_partition(
            begin_it,
            end_it,
            [&](T v) { return !pred(v); }
        );

        size_t count = std::distance(remaining_end, end_it);

        std::rotate(remaining_end, end_it, _data.end());

        _active_end = remaining_end;
        _fixed_end -= count;

        for (auto it = _fixed_end; it != _data.end(); ++it) {
            _policy.OnMoveToPricedOut(*it);
            _policy.OnCommit(*it);
        }

        return count;
    }

    // =====================================================
    // DEBUG VALIDATION
    // =====================================================

#ifdef _DEBUG
    void Validate() const {

        assert(_data.begin() <= _active_end);
        assert(_active_end <= _fixed_end);
        assert(_fixed_end <= _data.end());

        // Optional deeper validation (if Policy supports it)
        if constexpr (requires(Policy p, T v) { p.IsActive(v); }) {

            for (auto it = _data.begin(); it < _active_end; ++it)
                assert(_policy.IsActive(*it));

            for (auto it = _active_end; it < _fixed_end; ++it)
                assert(_policy.IsFixed(*it));

            for (auto it = _fixed_end; it < _data.end(); ++it)
                assert(_policy.IsPricedOut(*it));
        }
    }
#endif

private:
    SizeType ActiveOffset() const {
        return static_cast<SizeType>(_active_end - _data.begin());
    }

    SizeType FixedOffset() const {
        return static_cast<SizeType>(_fixed_end - _data.begin());
    }

    void RestorePartitionIterators(SizeType active_offset, SizeType fixed_offset) {
        _active_end = _data.begin() + active_offset;
        _fixed_end = _data.begin() + fixed_offset;
    }

    template <class Operation>
    void PreservePartitionIterators(Operation operation) {
        auto active_offset = ActiveOffset();
        auto fixed_offset = FixedOffset();

        operation();

        RestorePartitionIterators(active_offset, fixed_offset);
    }

    SizeType ErasedBeforeBoundary(
        SizeType first_offset,
        SizeType last_offset,
        SizeType erased_count,
        SizeType boundary_offset) const {

        if (boundary_offset <= first_offset)
            return 0;
        if (boundary_offset >= last_offset)
            return erased_count;
        return boundary_offset - first_offset;
    }
}; 

#endif
