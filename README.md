# A toy dependency injection framework for C++.

## Usage

Use the INJECT macro to declare a constructor to be used for injection. Parameters can be annotated with types using the ANNOTATED macro.

Example:

```
class MyType {
public:
  ANNOTATED(Type1, Type2)
  INJECT(MyType(Dep1 obj1, Dep2 obj2, Dep3 obj3)) {}
};
```

To bind objects, use one of the `bind<category>` functions. Retrieve a bound object manually using `inject`.

## Requirements
* Injection constructor must be public.
* Classes with an injection constructor must be final and move-constructible (copy-constructible is not a requirement except for `bindObject`).
* Injecting references is not supported (it may or may not compile and work properly). Use a shared_ptr instead.

## Best Practices
* To bind a singleton of type V, use `bindObject(shared_ptr<V>)` (if the singleton may differ based on some logic, use `bindSupplier(() -> shared_ptr<V>)`).
* To produce a new instance of V on every injection, use `bindSupplier(() -> unique_ptr<V>)`.
* To bind an interface Base to an implementation Child, use `bindClass<Base, Child>()`
