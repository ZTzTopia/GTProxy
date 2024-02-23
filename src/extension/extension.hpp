#pragma once
#include <unordered_map>
#include <ranges>

/**
 * @brief Provide a unique ID for each extension.
 *
 * @param uid The unique ID of the extension.
 */
#define PROVIDE_EXT_UID(uid)									\
	static constexpr std::size_t ExtensionUID = uid;			\
	std::size_t get_uid() override { return ExtensionUID; }

namespace extension {
/**
 * @brief Interface for extensions.
 *
 * This interface is used to provide a unique ID for each extension.
 * It is used to identify extensions in the Extensible class.
 */
struct IExtension {
	IExtension() = default;
	virtual ~IExtension()
	{
		for (const auto& method : std::views::values(methods_)) {
			::operator delete(method);
		}
	}

	/**
	 * @brief Get the unique ID of the extension.
	 *
	 * @return std::size_t The unique ID of the extension.
	 */
	virtual std::size_t get_uid() = 0;

	/**
	 * @brief Called when the extension is initialized.
	 *
	 */
	virtual void init() = 0;

	/**
	 * @brief Called every tick.
	 *
	 */
	virtual void tick() { }

	/**
	 * @brief Called when the extension is freed.
	 *
	 */
	virtual void free() = 0;

public:
	/**
	 * @brief Add a callable method to the extension.
	 *
	 * @tparam Callback The type of the callable method.
	 * @param name The name of the callable method.
	 * @param cb The callable method.
	 */
	template<typename Callback>
	void add_callable_method(const std::string& name, Callback cb)
	{
		auto f{ to_function(cb) };
		auto fn{ new decltype(f)(f) };
		methods_.emplace(name, fn);
	}

	/**
	 * @brief Call a method of the extension.
	 *
	 * @tparam R The return type of the method.
	 * @tparam Args The argument types of the method.
	 * @param name The name of the method.
	 * @param args The arguments of the method.
	 * @return R The return value of the method.
	 */
	template <typename R, typename... Args>
	R call_method(const std::string& name, Args... args)
	{
		const auto it{ methods_.find(name) };
		if (it == methods_.end()) {
			return R{};
		}

		auto fn = static_cast<std::function<R(Args...)>*>(it->second);
		return (*fn)(args...);
	}

private:
	/**
	 * @brief Get the function type of a callable object.
	 *
	 * @tparam Callback The type of the callable object.
	 */
	template <typename Callback>
	struct traits : traits<decltype(&Callback::operator())> {

	};

	/**
	 * @brief Traits for function (not member function) pointers.
	 *
	 * @tparam R The return type of the callable object.
	 * @tparam Args The argument types of the callable object.
	 */
	template <typename R, typename... Args>
	struct traits<R(*)(Args...)> {
		using fn = std::function<R(Args...)>;
	};

	/**
	 * @brief Traits for lambda functions.
	 *
	 * @tparam R The return type of the callable object.
	 * @tparam ClassType The class type of the callable object.
	 * @tparam Args The argument types of the callable object.
	 */
	template <typename ClassType, typename R, typename... Args>
	struct traits<R(ClassType::*)(Args...) const> {
		using fn = std::function<R(Args...)>;
	};

	/**
	 * @brief Traits for std::function.
	 *
	 * @tparam R The return type of the callable object.
	 * @tparam Args The argument types of the callable object.
	 */
	template <typename R, typename... Args>
	struct traits<std::function<R(Args...)>> {
		using fn = std::function<R(Args...)>;
	};

	/**
	 * @brief Traits for std::bind.
	 *
	 * @tparam R The return type of the callable object.
	 * @tparam ClassType The class type of the callable object.
	 * @tparam Args The argument types of the callable object.
	 */
	template <typename R, typename ClassType, typename... Args>
#ifdef _MSC_VER // MSVC
	struct traits<std::_Binder<std::_Unforced, R(ClassType::*)(Args...) const, ClassType*>> {
#else // GCC
	struct traits<std::_Bind<R(ClassType::*)(Args...) const>> {
#endif
		using fn = std::function<R(Args...)>;
	};

	/**
	 * @brief Convert a callable object to a function.
	 *
	 * @tparam Callback The type of the callable object.
	 */
	template <typename Callback>
	typename traits<Callback>::fn to_function(Callback& cb) {
		return static_cast<typename traits<Callback>::fn>(cb);
	}

private:
    std::unordered_map<std::string, void*> methods_;
};

/**
 * @brief Class that can be extended.
 *
 * This class is used to provide a way to extend a class.
 * It is used to extend the Core class.
 */
class Extensible {
public:
	virtual ~Extensible() { remove_extensions(); }

	/**
	 * @brief Add an extension to the class.
	 *
	 * @param ext The extension to add.
	 * @return true If the extension was added.
	 * @return false If the extension was not added.
	 */
	virtual bool add_extension(IExtension* ext)
	{
		if (!ext) {
			return false;
		}

		const auto it{ extensions_.find(ext->get_uid()) };
		if (it != extensions_.end() && it->second != ext) {
			remove_extension(ext->get_uid());
		}

		return extensions_.emplace(ext->get_uid(), ext).second;
	}

	/**
	 * @brief Get an extension by its unique ID.
	 *
	 * @param id The unique ID of the extension.
	 * @return IExtension* The extension.
	 */
	virtual IExtension* get_extension(const std::size_t id)
	{
		const auto it{ extensions_.find(id) };
		if (it == extensions_.end()) {
			return nullptr;
		}

		return it->second;
	}

	/**
	 * @brief Get an extension by its interface.
	 *
	 * @return IExtension* The extension.
	 */
	template <class T>
	T* query_extension()
	{
		static_assert(std::is_base_of_v<IExtension, T>, "Type must derive from IExtension");

		if (IExtension* extension{ get_extension(T::ExtensionUID) }) {
			return static_cast<T*>(extension);
		}

		return nullptr;
	}

	/**
	 * @brief Remove an extension by its object.
	 *
	 * @param ext The extension to remove.
	 * @return true If the extension was removed.
	 * @return false If the extension was not removed.
	 */
	virtual bool remove_extension(IExtension* ext)
	{
		const auto it{ extensions_.find(ext->get_uid()) };
		if (it == extensions_.end()) {
			return false;
		}

		if (it->second) {
			it->second->free();
		}

		extensions_.erase(it);
		return true;
	}

	/**
	 * @brief Remove an extension by its unique ID.
	 *
	 * @param id The unique ID of the extension.
	 * @return true If the extension was removed.
	 * @return false If the extension was not removed.
	 */
	virtual bool remove_extension(const std::size_t id)
	{
		const auto it{ extensions_.find(id) };
		if (it == extensions_.end()) {
			return false;
		}

		if (it->second) {
			it->second->free();
		}

		extensions_.erase(it);
		return true;
	}

private:
	void remove_extensions()
	{
		for (const auto& ext : std::views::values(extensions_)) {
			remove_extension(ext);
		}
	}

protected:
	std::unordered_map<std::size_t, IExtension*> extensions_;
};
}
