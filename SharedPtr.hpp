#ifndef CS540_SHAREDPTR_HPP
#define CS540_SHAREDPTR_HPP

#include<iostream>
#include<atomic>
#include<mutex>

#define DEBUG_STM(fmt) \
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<": "<<fmt<<std::endl
#define ERROR_STM(fmt) \
        std::cerr<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<": "<<fmt<<std::endl

namespace cs540{
   class SharedPtrHelper{
    public:
      virtual void incCountRef(){}
      virtual void getRefCount(){}
      virtual void callDest(){}
      virtual ~SharedPtrHelper(){}
  };
  
  template<class H>
  class SharedPtrHelper_2 : public SharedPtrHelper{
    public:
      typedef  H* ptr;
      typedef  int* atPtr;
      typedef  std::mutex* countMutex;
      
    private:
      ptr ptr_m;
      atPtr refCount_m;
      countMutex countMutex_m;
      
    public:
      SharedPtrHelper_2(H* h) : ptr_m(h), refCount_m(new int(1)), countMutex_m(new std::mutex){}
      //Prints the reference count on screen. This is for debug purposes
      void getRefCount(){DEBUG_STM((*this->refCount_m));}
      void incCountRef(){
        countMutex_m->lock();
        (*this->refCount_m)++;
        countMutex_m->unlock();
      }
      void callDest(){
        countMutex_m->lock();
        (*this->refCount_m)--;
        if(!(*this->refCount_m)){
          delete this;
          return;
        }
        countMutex_m->unlock();
      }
      //Destructor
      virtual ~SharedPtrHelper_2(){
        delete ptr_m;
        delete refCount_m;
        delete countMutex_m;
      }
  };
  
  template<typename T>
  class SharedPtr{
    public:
      typedef T* ptr;
      typedef SharedPtrHelper* refCtr;
      
      //class no-bound friend class
      template<typename>
      friend class SharedPtr;
      
    private:
      ptr ptr_m;   //pointer of type T
      refCtr rCtr_m;  //Object of helper class to count no of references and
                      // manage the object
      
    public:
      SharedPtr() : ptr_m(nullptr), rCtr_m(nullptr){
      }
      template<typename U> explicit SharedPtr(U *p) : ptr_m(p), rCtr_m(new SharedPtrHelper_2<U>(p)){
      }
      SharedPtr(const SharedPtr& r) : ptr_m(r.ptr_m), rCtr_m(r.rCtr_m){
        if(rCtr_m!=nullptr)
          rCtr_m->incCountRef();
      }
      template<typename U>SharedPtr(const SharedPtr<U>& r) : ptr_m(r.ptr_m), rCtr_m(r.rCtr_m){
        if(rCtr_m!=nullptr)
          rCtr_m->incCountRef();
      }
      SharedPtr(SharedPtr&& r) : ptr_m(std::move(r.ptr_m)), rCtr_m(std::move(r.rCtr_m)){
        r.ptr_m = nullptr;
        r.rCtr_m = nullptr;
      }
      template<typename U>SharedPtr(const SharedPtr<U>&& r) : ptr_m(std::move(r.ptr_m)), rCtr_m(std::move(r.rCtr_m)){
        r.ptr_m = nullptr;
        r.rCtr_m = nullptr;
      }
      ~SharedPtr(){
        this->ptr_m=nullptr;
        if(rCtr_m!=nullptr){
          rCtr_m->callDest();
        }
        else
          delete this->rCtr_m;
      }
      
      T *get() const{
        return ptr_m;
      }
      
      template<typename U> void reset(U *p){
        if(rCtr_m!=nullptr)
          this->rCtr_m->callDest();
        this->rCtr_m = new SharedPtrHelper_2<U>(p);
        this->ptr_m=p;
      }
      
      void reset(std::nullptr_t){
        if(rCtr_m!=nullptr)
        this->rCtr_m->callDest();
        this->ptr_m=nullptr;
        this->rCtr_m=nullptr;
      }
      
      SharedPtr& operator=(SharedPtr&& r){
        this->rCtr_m->callDest();
        this->ptr_m = std::move(r.ptr_m);
        this->rCtr_m = std::move(r.rCtr_m);
        r.ptr_m=nullptr;
        return *this;
      }
      
      template<typename U> SharedPtr& operator=(SharedPtr<U>&& r){
        this->rCtr_m->callDest();
        this->rCtr_m = new SharedPtrHelper_2<U>(r);
        this->ptr_m = std::move(r.ptr_m);
        r.ptr_m = nullptr;
        return *this;
      }
      
      SharedPtr& operator=(const SharedPtr& r){
        //if(this == &r) return *this;     //This also works
        if( this->get() == r.get()) return *this;
        if(rCtr_m!=nullptr){
          rCtr_m->callDest();
        }
        this->ptr_m = r.ptr_m;
        this->rCtr_m = r.rCtr_m;
        if(rCtr_m!=nullptr)
          r.rCtr_m->incCountRef();
        return *this;
      }
      
      template<typename U> SharedPtr& operator=(const SharedPtr<U>& r){
        if( this->get() == r.get()) return *this;
        if(rCtr_m!=nullptr){
          rCtr_m->callDest();
        }
        this->ptr_m = r.ptr_m;
        this->rCtr_m = r.rCtr_m;
        if(rCtr_m!=nullptr)
        this->rCtr_m->incCountRef();
        return *this;
      }
      
      T& operator*() const{
        return *(this->ptr_m);
      }
      
      T *operator->() const{
        return this->ptr_m;
      }
      
      explicit operator bool() const{
        return (ptr_m != nullptr) ? true : false;
      }
  };
  
  //== operator overloading
  template<typename T1, typename T2>
  bool operator==(const SharedPtr<T1>& A, const SharedPtr<T2>& B){
    return (A.get() == B.get());
  }
  
  //== operator overloading
  template<typename T1>
  bool operator==(const SharedPtr<T1>& A, std::nullptr_t){
    return (A.get() == nullptr);
  }
  
  //== operator overloading
  template<typename T1>
  bool operator==(std::nullptr_t, const SharedPtr<T1>& A){
    return (A.get() == nullptr);
  }
  
  // != operator overloading
  template<typename T1, typename T2>
  bool operator!=(const SharedPtr<T1>& A, const SharedPtr<T2>& B){
    return (A.get() != B.get());
  }
  
  // != operator overloading
  template<typename T1>
  bool operator!=(const SharedPtr<T1>& A, std::nullptr_t){
    return (A.get() != nullptr);
  }
  
  // != operator overloading
  template<typename T1>
  bool operator!=(std::nullptr_t, const SharedPtr<T1>& A){
    return (A.get() != nullptr);
  }
  
}//End of namespace

#endif //#ifndef CS540_SHAREDPTR_HPP

