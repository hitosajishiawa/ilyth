#ifndef ILYTH_UTILS_SINGLETON_H
#define ILYTH_UTILS_SINGLETON_H

namespace ilyth
{

/**
 * @warning You may need no *Singleton Pattern* at all! Read the following
 * essays and think twice:
 *  - <docs/archive/singleton1.md>
 *  - <docs/archive/singleton2.md>
 *
 * @brief Meyers' Singleton as a CRTP base: exactly one lazily created @p T per
 * process, initialized on first access.
 *
 * @details Recipe for a derived type:
 *  1. inherit with the derived type itself as the template argument;
 *  2. keep its constructor and destructor private;
 *  3. befriend `Singleton<Derived>`.
 *
 * ```
 * class Example final : public ilyth::Singleton<Example>
 * {
 *     friend class ilyth::Singleton<Example>;
 *
 * private:
 *     Example()  = default;
 *     ~Example() = default;
 *
 * public:
 *     void greet() const;
 * };
 *
 * // Usage:
 * ilyth::Singleton<Example>::GetInstance().greet();
 * Example::GetInstance().greet();  // equivalent to the line above
 * ```
 *
 * @note Limitations of the Meyers' idiom:
 *  - Destruction order: the instance dies at exit, in an order unrelated to
 *    its users. A static object that touches the singleton while it is being
 *    destroyed may see an already destroyed instance. Where that matters, leak
 *    the instance on purpose (`static auto* inst = new T(); return *inst;`).
 *  - Recursive initialization: calling `GetInstance()` from @p T 's own
 *    constructor is undefined behavior.
 *
 * @note You are free to not depend on this template at all: hand-writing the
 * pattern fits more flexible scenarios. For example, a function-local static
 * behind a free accessor:
 * ```
 * // in some .cpp
 * namespace {
 * auto example() -> Example&
 * {
 *     static Example inst;
 *     return inst;
 * }
 * }
 * ```
 *
 * Reach for this template only when several types need it and you want the
 * "private constructor + non-copyable" rule enforced by a base class.
 * Prefer the hand-written form when any of the following applies:
 *  - the accessor must return something other than `T&` (a `shared_ptr`, a
 *    plain handle, ...);
 *  - it has to stay hidden inside a translation unit;
 *  - the object's lifetime is not "until exit".
 *
 * @tparam T The derived type; must name itself as the template argument.
 */
template <class T>
class Singleton
{
protected:
    Singleton()  = default;
    ~Singleton() = default;

public:
    Singleton(const Singleton&)            = delete;
    Singleton(Singleton&&)                 = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&)      = delete;

    /// @brief The one and only instance, created on first call.
    [[nodiscard]] static auto GetInstance() -> T&
    {
        static T inst;
        return inst;
    }
};

}  // namespace ilyth

#endif
