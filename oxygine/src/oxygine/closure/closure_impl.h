// general template
template <class F>
class Closure;

// forward declaration of specialization
template <TEMPLATE_PARAM_LIST>
class Closure<R(PARAM_TYPE_LIST)>;

namespace detail {
extern uint64_t NEXT_FINGERPRINT;
template <typename Clazz>
std::string GetPtrName(Clazz*);
namespace CLOSURE_NUM {
template <TEMPLATE_PARAM_LIST>
class Callback {
  public:
   Callback(uint8_t t) : type(t) {}
   virtual ~Callback() {}
   virtual bool equals(const Callback<R PARAM_TYPE_LIST_COMMA>& c) const {
      return this->type == c.type;
   }
   virtual R execute(PARAM_TYPE_LIST) const = 0;
   virtual Callback<R PARAM_TYPE_LIST_COMMA>* copy() const = 0;
   virtual bool owned(const void*) const { return false; }

  protected:
   uint8_t type;
};
template <TEMPLATE_PARAM_LIST>
class FunctionCallback : public Callback<R PARAM_TYPE_LIST_COMMA> {
  public:
   using FuncType = R (*)(PARAM_TYPE_LIST);

   FunctionCallback(FuncType func) : Callback<R PARAM_TYPE_LIST_COMMA>(0x1), function(func) {}
   virtual bool equals(const Callback<R PARAM_TYPE_LIST_COMMA>& c) const override {
      return Callback<R PARAM_TYPE_LIST_COMMA>::equals(c) && ((const FunctionCallback<R PARAM_TYPE_LIST_COMMA>&)c).function == this->function;
   }
   virtual R execute(PARAM_FORM_ARG_LIST) const override {
      return (*this->function)(PARAM_ARG_LIST);
   }
   virtual Callback<R PARAM_TYPE_LIST_COMMA>* copy() const override {
      return new FunctionCallback<R PARAM_TYPE_LIST_COMMA>(this->function);
   }
   FuncType function;
};
template <TEMPLATE_PARAM_LIST>
class CallableCallback : public Callback<R PARAM_TYPE_LIST_COMMA> {
  public:
   using FuncType = std::function<R(PARAM_TYPE_LIST)>;

   CallableCallback(FuncType func) : Callback<R PARAM_TYPE_LIST_COMMA>(0x2), function(func) {}
   virtual bool equals(const Callback<R PARAM_TYPE_LIST_COMMA>& c) const override {
      return false;
   }
   virtual R execute(PARAM_FORM_ARG_LIST) const override {
      return (this->function)(PARAM_ARG_LIST);
   }
   virtual Callback<R PARAM_TYPE_LIST_COMMA>* copy() const override {
      return new CallableCallback<R PARAM_TYPE_LIST_COMMA>(this->function);
   }
   FuncType function;
};

template <typename Clazz, TEMPLATE_PARAM_LIST>
class PtrCallback : public Callback<R PARAM_TYPE_LIST_COMMA> {
  public:
   using FuncType = R (Clazz::*)(PARAM_TYPE_LIST);

   PtrCallback(Clazz* ptr, FuncType func) : Callback<R PARAM_TYPE_LIST_COMMA>(0x3), target(ptr), function(func) {}
   virtual bool equals(const Callback<R PARAM_TYPE_LIST_COMMA>& c) const override {
      if (!Callback<R PARAM_TYPE_LIST_COMMA>::equals(c)) return false;
      const PtrCallback<Clazz, R PARAM_TYPE_LIST_COMMA>& o = (const PtrCallback<Clazz, R PARAM_TYPE_LIST_COMMA>&)c;
      return this->target == o.target && this->function == o.function;
   }
   virtual R execute(PARAM_FORM_ARG_LIST) const override {
      return (this->target->*(this->function))(PARAM_ARG_LIST);
   }
   virtual Callback<R PARAM_TYPE_LIST_COMMA>* copy() const override {
      return new PtrCallback<Clazz, R PARAM_TYPE_LIST_COMMA>(this->target, this->function);
   }
   virtual bool owned(const void* ptr) const override { return this->target == ptr; }

   Clazz* target;
   FuncType function;
};

template <typename Clazz, TEMPLATE_PARAM_LIST>
class ConstPtrCallback : public Callback<R PARAM_TYPE_LIST_COMMA> {
  public:
   using FuncType = R (Clazz::*)(PARAM_TYPE_LIST) const;

   ConstPtrCallback(const Clazz* ptr, FuncType func) : Callback<R PARAM_TYPE_LIST_COMMA>(0x4), target(ptr), function(func) {}
   virtual bool equals(const Callback<R PARAM_TYPE_LIST_COMMA>& c) const override {
      if (!Callback<R PARAM_TYPE_LIST_COMMA>::equals(c)) return false;
      const ConstPtrCallback<Clazz, R PARAM_TYPE_LIST_COMMA>& o = (const ConstPtrCallback<Clazz, R PARAM_TYPE_LIST_COMMA>&)c;
      return this->target == o.target && this->function == o.function;
   }
   virtual R execute(PARAM_FORM_ARG_LIST) const override {
      return (this->target->*(this->function))(PARAM_ARG_LIST);
   }
   virtual Callback<R PARAM_TYPE_LIST_COMMA>* copy() const override {
      return new ConstPtrCallback<Clazz, R PARAM_TYPE_LIST_COMMA>(this->target, this->function);
   }
   virtual bool owned(const void* ptr) const override { return this->target == ptr; }
   const Clazz* target;
   FuncType function;
};
}  // namespace CLOSURE_NUM
}  // namespace detail

template <TEMPLATE_PARAM_LIST>
class Closure<R(PARAM_TYPE_LIST)> {
  public:
   Closure() {}
   Closure(std::nullptr_t) {}
   Closure(R (*func)(PARAM_TYPE_LIST)) {
      if (func) {
         this->callback = new detail::CLOSURE_NUM::FunctionCallback<R PARAM_TYPE_LIST_COMMA>(func);
         this->fingerprint = ++detail::NEXT_FINGERPRINT;
      }
   }
   template <typename Clazz>
   Closure(Clazz* ptr, R (Clazz::*func)(PARAM_TYPE_LIST)) {
      this->callback = new detail::CLOSURE_NUM::PtrCallback<Clazz, R PARAM_TYPE_LIST_COMMA>(ptr, func);
      this->fingerprint = ++detail::NEXT_FINGERPRINT;
   }
   template <typename Clazz>
   Closure(const Clazz* ptr, R (Clazz::*func)(PARAM_TYPE_LIST) const) {
      this->callback = new detail::CLOSURE_NUM::ConstPtrCallback<Clazz, R PARAM_TYPE_LIST_COMMA>(ptr, func);
      this->fingerprint = ++detail::NEXT_FINGERPRINT;
   }
   template <typename Lambda>
   Closure(Lambda l) {
      std::function<R(PARAM_TYPE_LIST)> func(l);
      this->callback = new detail::CLOSURE_NUM::CallableCallback<R PARAM_TYPE_LIST_COMMA>(func);
      this->fingerprint = ++detail::NEXT_FINGERPRINT;
   }
   ~Closure() {
      if (this->callback) {
         delete this->callback;
      }
   }
   Closure(const Closure<R(PARAM_TYPE_LIST)>& o) {
      if (o.callback)
         this->callback = o.callback->copy();
      else
         this->callback = nullptr;
      this->fingerprint = o.fingerprint;
   }
   Closure<R(PARAM_TYPE_LIST)>& operator=(const Closure<R(PARAM_TYPE_LIST)>& o) {
      if (o.callback)
         this->callback = o.callback->copy();
      else
         this->callback = nullptr;
      this->fingerprint = o.fingerprint;
      return *this;
   }
   template <typename Clazz>
   bool owned(const Clazz* ptr) const {
      return this->callback && this->callback->owned(ptr);
   }

   R operator()(PARAM_FORM_ARG_LIST) {
      if (this->callback)
         return this->callback->execute(PARAM_ARG_LIST);
      else
         throw std::invalid_argument("null callback called");
   }
   explicit operator bool() const {
      return this->callback != nullptr;
   }

   bool operator!() const {
      return !this->callback;
   }

   bool operator==(const Closure& c) const {
      if (c.fingerprint == this->fingerprint || c.callback == this->callback) return true;
      if (!c.callback || !this->callback) return false;
      return c.callback->equals(*this->callback);
   }
   uint64_t getFingerprint() const { return fingerprint; }

  private:
   uint64_t fingerprint = 0;
   detail::CLOSURE_NUM::Callback<R PARAM_TYPE_LIST_COMMA>* callback = NULL;
};

namespace detail {
namespace CLOSURE_NUM {

template <class T, TEMPLATE_PARAM_LIST>
struct CreateClosureHelper {
   template <R (T::*p_mem)(PARAM_TYPE_LIST)>
   Closure<R(PARAM_TYPE_LIST)> Init(T* p_this) {
      Closure<R(PARAM_TYPE_LIST)> c(p_this, p_mem);
      return c;
   }
};

template <class T, TEMPLATE_PARAM_LIST>
struct CreateClosureHelperConst {
   template <R (T::*p_mem)(PARAM_TYPE_LIST) const>
   Closure<R(PARAM_TYPE_LIST)> Init(const T* p_this) {
      Closure<R(PARAM_TYPE_LIST)> c(p_this, p_mem);
      return c;
   }
};
}  // namespace CLOSURE_NUM

// helper function, to deduce return and parameters types of given pointer to member function
template <class T, TEMPLATE_PARAM_LIST>
CLOSURE_NUM::CreateClosureHelper<T, R PARAM_TYPE_LIST_COMMA> CreateClosure(R (T::*)(PARAM_TYPE_LIST)) {
   return CLOSURE_NUM::CreateClosureHelper<T, R PARAM_TYPE_LIST_COMMA>();
}

// helper function, to deduce return and parameters types of given pointer to member const function
template <class T, TEMPLATE_PARAM_LIST>
CLOSURE_NUM::CreateClosureHelperConst<T, R PARAM_TYPE_LIST_COMMA> CreateClosure(R (T::*)(PARAM_TYPE_LIST) const) {
   return CLOSURE_NUM::CreateClosureHelperConst<T, R PARAM_TYPE_LIST_COMMA>();
}
}  // namespace detail

#undef TEMPLATE_PARAM_LIST
#undef PARAM_TYPE_LIST
#undef PARAM_TYPE_LIST_COMMA
#undef PARAM_FORM_ARG_LIST
#undef PARAM_FORM_ARG_LIST_COMMA
#undef PARAM_ARG_LIST
#undef PARAM_ARG_LIST_COMMA
#undef CLOSURE_NUM
