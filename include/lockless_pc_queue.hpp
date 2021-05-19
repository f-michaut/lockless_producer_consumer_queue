#include <atomic> // TODO use atomic
#include <cstddef>

// TODO maybe do not work with pointers to T and just T directly ?
// No need for the caller to delete stuff

namespace Lockless {
    /*
     * Implementation of a Lockless Producer Consumer Queue (PC_Queue)
     */
    template <class T> class PC_Queue {
        private:
            struct ListElems {
                T m_value;
                ListElems *m_next = nullptr;
            };
        public:
            PC_Queue () :
                m_head(nullptr), m_tail(nullptr), m_consum_head(nullptr)
            {};

            ~PC_Queue() {
                // TODO FIXME Oh shit if one thread destroy the queue before the other it's gonna be fun
            };

            // TODO create copy / move assignements operators

            std::size_t size() const noexcept { // TODO hard test this function
                std::size_t res = m_size - m_consumed;

                if (m_tail == m_consum_head)
                    return 0;
                return res;
            };

            void produce(const T arg) {
                auto tmp = new ListElems();

                tmp->m_value = arg;
                if (m_tail == nullptr) {
                    m_head = tmp;
                } else {
                    m_tail->m_next = tmp;
                }
                m_tail = tmp;
                m_size++;
            };

            T consume() noexcept {
                T res;

                if (m_head == nullptr)
                    return nullptr;
                if (m_head == m_consum_head && m_head->m_next == nullptr)
                    return nullptr;
                res = m_head->m_value;
                m_consum_head = m_head;
                if (m_head->next != nullptr) {
                    auto tmp = m_head;
                    m_head = m_head->next;
                    delete tmp;
                }
                m_consumed++;
                return res;
            };

        private:
            ListElems *m_head;
            ListElems *m_tail;
            ListElems *m_consum_head;

            std::size_t m_size = 0;
            std::size_t m_consumed = 0;
    };
}
