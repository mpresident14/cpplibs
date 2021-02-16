## Parser Generator Requirements:
1. The top-level object must be move- or copy-constructible. Any objects passed as arguments to a constructor expression are subsequently destroyed with the default destructor for its type (__NOT__ the custom one provided by the user), so they must be copied or moved. Since the destruction is shallow, the constructor expression assumes ownership of any pointers.
