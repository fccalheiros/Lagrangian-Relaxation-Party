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
// - begin <= active_end <= fixed_end <= end
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
    Container data;

    Iterator active_end; // end of ACTIVE region
    Iterator fixed_end;  // end of FIXED region (start of PRICED_OUT)

    Policy policy;

public:

    PartitionedVector() {
        InitEmpty();
    }

    // =====================================================
    // Initialization
    // =====================================================

    void InitEmpty() {
        active_end = data.begin();
        fixed_end = data.begin();
    }

    void InitAllActive() {
        active_end = data.end();
        fixed_end = data.end();
    }

    // =====================================================
    // Access (compatibility layer)
    // =====================================================

    Iterator begin() { return data.begin(); }
    Iterator end() { return data.end(); }
    ConstIterator begin() const { return data.begin(); }
    ConstIterator end() const { return data.end(); }
    ConstIterator cbegin() const { return data.cbegin(); }
    ConstIterator cend() const { return data.cend(); }

    Reference operator[](SizeType pos) { return data[pos]; }
    ConstReference operator[](SizeType pos) const { return data[pos]; }

    Reference at(SizeType pos) { return data.at(pos); }
    ConstReference at(SizeType pos) const { return data.at(pos); }

    Reference front() { return data.front(); }
    ConstReference front() const { return data.front(); }

    Reference back() { return data.back(); }
    ConstReference back() const { return data.back(); }

    bool empty() const { return data.empty(); }
    SizeType size() const { return data.size(); }
    SizeType capacity() const { return data.capacity(); }

    void reserve(SizeType new_cap) {
        PreservePartitionIterators([&]() {
            data.reserve(new_cap);
        });
    }

    void shrink_to_fit() {
        PreservePartitionIterators([&]() {
            data.shrink_to_fit();
        });
    }

    void push_back(ConstReference value) {
        PreservePartitionIterators([&]() {
            data.push_back(value);
        });
    }

    void push_back(ValueType&& value) {
        PreservePartitionIterators([&]() {
            data.push_back(std::move(value));
        });
    }

    template <class... Args>
    Reference emplace_back(Args&&... args) {
        PreservePartitionIterators([&]() {
            data.emplace_back(std::forward<Args>(args)...);
        });
        return data.back();
    }

    Iterator erase(Iterator pos) {
        auto active_offset = ActiveOffset();
        auto fixed_offset = FixedOffset();
        auto erase_offset = static_cast<SizeType>(pos - data.begin());

        Iterator next = data.erase(pos);

        if (erase_offset < active_offset)
            --active_offset;
        if (erase_offset < fixed_offset)
            --fixed_offset;

        RestorePartitionIterators(active_offset, fixed_offset);
        return data.begin() + (next - data.begin());
    }

    Iterator erase(Iterator first, Iterator last) {
        auto active_offset = ActiveOffset();
        auto fixed_offset = FixedOffset();
        auto first_offset = static_cast<SizeType>(first - data.begin());
        auto last_offset = static_cast<SizeType>(last - data.begin());
        auto erased_count = last_offset - first_offset;

        Iterator next = data.erase(first, last);

        active_offset -= ErasedBeforeBoundary(first_offset, last_offset, erased_count, active_offset);
        fixed_offset -= ErasedBeforeBoundary(first_offset, last_offset, erased_count, fixed_offset);

        RestorePartitionIterators(active_offset, fixed_offset);
        return data.begin() + (next - data.begin());
    }

    void clear() {
        data.clear();
        InitEmpty();
    }

    // =====================================================
    // Counts
    // =====================================================

    int ActiveCount() const {
        return static_cast<int>(active_end - data.begin());
    }

    int FixedCount() const {
        return static_cast<int>(fixed_end - active_end);
    }

    int PricedOutCount() const {
        return static_cast<int>(data.end() - fixed_end);
    }

    // =====================================================
    // Ranges
    // =====================================================

    std::pair<Iterator, Iterator> ActiveRange() {
        return { data.begin(), active_end };
    }

    std::pair<Iterator, Iterator> FixedRange() {
        return { active_end, fixed_end };
    }

    std::pair<Iterator, Iterator> PricedOutRange() {
        return { fixed_end, data.end() };
    }

    // =====================================================
    // SINGLE ELEMENT TRANSITIONS (O(1))
    // =====================================================

    void MoveActiveToFixed(Iterator it) {
        policy.OnMoveToFixed(*it);
        active_end--;
        std::iter_swap(it, active_end);
    }

    void MoveFixedToActive(Iterator it) {
        policy.OnMoveToActive(*it);
        std::iter_swap(it, active_end);
        active_end++;
    }

    void MoveActiveToPricedOut(Iterator it) {
        policy.OnMoveToPricedOut(*it);

        // Step 1: move to end of ACTIVE
        active_end--;
        std::iter_swap(it, active_end);

        // Step 2: move to end of FIXED (i.e., into PRICED_OUT)
        fixed_end--;
        std::iter_swap(active_end, fixed_end);
    }

    void MovePricedOutToActive(Iterator it) {
        policy.OnMoveToActive(*it);

        // Step 1: bring element to FIXED boundary
        std::iter_swap(it, fixed_end);
        fixed_end++;

        // Step 2: bring element to ACTIVE
        std::iter_swap(fixed_end - 1, active_end);
        active_end++;
    }


    void MoveFixedToPricedOut(Iterator it) {
        policy.OnMoveToPricedOut(*it);
        fixed_end--;
        std::iter_swap(it, fixed_end);
    }

    void MovePricedOutToFixed(Iterator it) {
        policy.OnMoveToFixed(*it);
        std::iter_swap(it, fixed_end);
        fixed_end++;
    }

    // =====================================================
    // BATCH: PRICED_OUT → ACTIVE
    // (equivalent to CommitPriceIn)
    // =====================================================

    template <class Predicate>
    size_t CommitPriceIn(Predicate pred) {

        auto first = fixed_end;
        auto last = data.end();

		auto selected_end = std::stable_partition(first, last, pred);

        std::rotate(active_end, first, selected_end);

        size_t count = std::distance(first, selected_end);

        active_end += count;
        fixed_end += count;

        for (auto it = active_end - count; it != active_end; ++it) {
            policy.OnMoveToActive(*it);
            policy.OnCommit(*it);
        }

        return count;
    }

    // =====================================================
    // BATCH: ACTIVE → PRICED_OUT
    // (equivalent to CommitPriceOut)
    // =====================================================

    template <class Predicate>
    size_t CommitPriceOut(Predicate pred) {

        auto begin_it = data.begin();
        auto end_it = active_end;

        auto remaining_end = std::stable_partition(
            begin_it,
            end_it,
            [&](T v) { return !pred(v); }
        );

        size_t count = std::distance(remaining_end, end_it);

        std::rotate(remaining_end, end_it, data.end());

        active_end = remaining_end;
        fixed_end -= count;

        for (auto it = fixed_end; it != data.end(); ++it) {
            policy.OnMoveToPricedOut(*it);
            policy.OnCommit(*it);
        }

        return count;
    }

    // =====================================================
    // DEBUG VALIDATION
    // =====================================================

#ifdef _DEBUG
    void Validate() const {

        assert(data.begin() <= active_end);
        assert(active_end <= fixed_end);
        assert(fixed_end <= data.end());

        // Optional deeper validation (if Policy supports it)
        if constexpr (requires(Policy p, T v) { p.IsActive(v); }) {

            for (auto it = data.begin(); it < active_end; ++it)
                assert(policy.IsActive(*it));

            for (auto it = active_end; it < fixed_end; ++it)
                assert(policy.IsFixed(*it));

            for (auto it = fixed_end; it < data.end(); ++it)
                assert(policy.IsPricedOut(*it));
        }
    }
#endif

private:
    SizeType ActiveOffset() const {
        return static_cast<SizeType>(active_end - data.begin());
    }

    SizeType FixedOffset() const {
        return static_cast<SizeType>(fixed_end - data.begin());
    }

    void RestorePartitionIterators(SizeType active_offset, SizeType fixed_offset) {
        active_end = data.begin() + active_offset;
        fixed_end = data.begin() + fixed_offset;
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
