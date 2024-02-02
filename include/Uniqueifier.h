// =====================================================================================
//
//       Filename:  Uniqueifier.h
//
//    Description:  Wrapper class to disambiguate common types 
//
//        Version:  1.0
//        Created:  02/02/2024 02:59:28 PM
//       Revision:  none
//       License:  GNU General Public License v3
//
//         Author:  David P. Riedel (), driedel@cox.net
//   Organization:  
//
// =====================================================================================

    // thanks to Jonathan Boccara of fluentcpp.com for his articles on
    // Strong Types and the NamedType library.
    //
    // this code is a simplified and somewhat stripped down version of his.

    // =====================================================================================
    //        Class:  UniqType
    //  Description: Provides a wrapper which makes embedded common data types distinguisable 
    // =====================================================================================

    template <typename T, typename Uniqueifier>
    class UniqType
    {
    public:
        // ====================  LIFECYCLE     ======================================= 

        UniqType() requires std::is_default_constructible_v<T>
            : value_{} {}

        UniqType(const UniqType<T, Uniqueifier>& rhs) requires std::is_copy_constructible_v<T>
            : value_{rhs.value_} {}

        explicit UniqType(T const& value) requires std::is_copy_constructible_v<T>
            : value_{value} {}

        UniqType(UniqType<T, Uniqueifier>&& rhs) requires std::is_move_constructible_v<T>
            : value_(std::move(rhs.value_)) {}
        
        explicit UniqType(T&& value) requires std::is_move_constructible_v<T>
            : value_(std::move(value)) {}

        // ====================  ACCESSORS     ======================================= 

        // not needed because we have assignment operators
        T& get() { return value_; }
        const T& get() const { return value_; }

        // ====================  MUTATORS      ======================================= 

        // ====================  OPERATORS     ======================================= 

        UniqType& operator=(const UniqType<T, Uniqueifier>& rhs) requires std::is_copy_assignable_v<T>
        {
            if (this != &rhs)
            {
                value_ = rhs.value_;
            }
            return *this;
        }
        UniqType& operator=(const T& rhs) requires std::is_copy_assignable_v<T>
        {
            if (&value_ != &rhs)
            {
                value_ = rhs;
            }
            return *this;
        }
        UniqType& operator=(UniqType<T, Uniqueifier>&& rhs) requires std::is_move_assignable_v<T>
        {
            if (this != &rhs)
            {
                value_ = std::move(rhs.value_);
            }
            return *this;
        }
        UniqType& operator=(T&& rhs) requires std::is_move_assignable_v<T>
        {
            if (&value_ != &rhs)
            {
                value_ = std::move(rhs);
            }
            return *this;
        }

    protected:
        // ====================  METHODS       ======================================= 

        // ====================  DATA MEMBERS  ======================================= 

    private:
        // ====================  METHODS       ======================================= 

        // ====================  DATA MEMBERS  ======================================= 
        
        T value_;

    }; // -----  end of class UniqType  ----- 

