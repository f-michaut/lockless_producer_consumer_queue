#include <atomic>
#include <cstddef>
#include <cstdint> // std::uintptr_t is optional... Maybe use something else ?
#include <memory> // We use smart ptrs to be able to return null values from the
                  // consume function (we could have thrown exceptions,
                  // but returning nulls seems more appropriate for that)

namespace Lockless {
    /*
     * Implementation of a Lockless Producer Consumer Queue (PC_Queue)
     * The idea was to implement a sort of linked list, with 1 thread (the producer)
     * always writting to the tail, and one thread (the consumer) always reading from the
     * head. The producer will also free the head if the head is not the tail (if it is,
     * it will wait until it's not before deleting it).
     */
    template <class T> class PC_Queue {
        private:
            struct ListElems {
                T m_value;
                std::atomic_uintptr_t m_next = 0;
                // ListElems *m_next = nullptr;
            };
        public:
            PC_Queue () :
                m_head(0), m_tail(nullptr), m_consum_head(nullptr)
            {};

            ~PC_Queue() {
                // TODO FIXME Oh shit if one thread destroy the queue before the other it's gonna be fun
                auto curr = (ListElems *)m_head.load();

                while (curr) {
                    auto next = (ListElems *)curr->m_next.load();

                    delete curr;
                    curr = next;
                }
            };

            // TODO create copy / move assignements operators

            std::size_t size() const noexcept { // TODO hard test this function
                if (m_tail == m_consum_head)
                    return 0; // Check if m_size is already 0 ?
                return m_size;
            };

            void produce(const T &arg) {
                auto tmp = new ListElems();

                tmp->m_value = arg;
                if (m_tail == 0) {
                    // This means we are producing the first element
                    // Thus, we need to update the head for the other thread
                    // (no data race since the other thread will read this value,
                    // and as long as it is still 0, it won't do anything)
                    // Furthermore, the head is an atomic ptr, so the
                    // assignements/read are garenteed to be atomics,
                    // so the other thread will only read this value once it's
                    // fully writen even on weird processor architecture
                    m_head = (std::uintptr_t)tmp;
                } else {
                    // If the queue already has elements inserted, we
                    // DO NOT use the head anymore, since the other thread might
                    // have deleted it. We use the tail since we know the other
                    // thread won't change it. In the event of the tail being
                    // equal to the head, we know that the other thread will not
                    // delete it, and will wait for the head to have a m_next
                    // value. Since it is also a atomic ptr, we know that the
                    // other thread will either see the value empty (before this
                    // line has been executed) or with a value (after this line
                    // has been executed). In the latter case, even if the head
                    // is deleted by the other thread it's fine since we won't
                    // acces it after.
                    m_tail->m_next = (std::uintptr_t)tmp;
                }
                m_tail = tmp;
                // size is a atomic int, we can increment it wihtout issues
                m_size++;
            };

            std::shared_ptr<T> consume() {
                T res;
                auto head = (ListElems *)m_head.load();
                std::uintptr_t next;

                // We load head at the begining of the function. The head is
                // modified only once by the other thread : when the first item
                // is inserted. So head will either be null, because it hasn't
                // been written to yet, or if it has a value, then we can safely
                // assume that the other thread won't touch it anymore and we
                // can use it however we want.

                // This is the first case, when no value has been written yet,
                // we just return an empty value since there is nothing to
                // consume yet.
                if (head == nullptr)
                    return nullptr;
                // If head is not null, we load it's next element. So at this
                // point, since it's also an aotmic pointer we will either get a
                // null or a value. In the case of a null, that just means that
                // the producer thread hasn't produced a next value yet, and we
                // just have to wait. But there is a catch here : if the other
                // thread hasn't writen the next value yet, we can't delete
                // this element yet (because even tho the other thread will use
                // the tail, both head and tail point to the same element if the
                // head doesn't have a next item).
                // But we would like to consume the item anyway. This is where
                // the consum_head comes into play.
                next = head->m_next.load();
                // If head is equal to consume_head, that means that we already
                // processed this head, and we are waiting for a next element to
                // consume. So if next is still null, well we can just return an
                // empty element because the queue is empty (there is an item
                // that we already consumed, we don't want to consume it again)
                if (head == m_consum_head && next == 0)
                    return nullptr;
                // So if head is equal to consum head but there is a next,
                // then we want to remove this head (since we already consumed
                // it) and continue with the next value.
                if (head == m_consum_head) {
                    m_head = next;
                    delete head;
                    head = (ListElems *)next;
                }
                // We know the value is not going to change, since the producer
                // only add items in the queue, so it's safe to not have a
                // atomic pointer here, because the value is going to be written
                // in the struct before the struct pointer is written to the
                // head variable.
                res = head->m_value;
                m_consum_head = head; // Updating the last consumed head
                if (head->m_next != 0) {
                    // If there is more elements in the queue we can move the
                    // head to the next element, since we are the only one using
                    // the head pointer (the producer thread always use the last
                    // element of the queue, and if the head has a next pointer,
                    // well the head is not the last element so we can delete it)
                    m_head = (std::uintptr_t)head->m_next;
                    delete head;
                }
                // size is a atomic int, we can decrement it wihtout issues
                m_size--;
                return std::make_shared<T>(res);
            };

        private:
            std::atomic_uintptr_t m_head;
            ListElems *m_tail;
            ListElems *m_consum_head;

            std::atomic_size_t m_size = 0;
    };
}
