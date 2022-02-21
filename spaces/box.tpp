//
// Created by dewe on 10/29/21.
//

namespace gym::space{

    template<typename T>
    Box<T>::Box(vector<Range<T > > range, Shape const& shape):Space(shape),m_Range(range){

        if(dim(shape) == 0){
            throw std::runtime_error("a zero dim m_Shape or scalar m_Shape is not allowed");
        }

        if(dim(shape) > 1){
            throw std::runtime_error("Initializing a Box m_Shape with a vectorized range "
                                     "requires that a m_Shape has only one dim");
        }
    }

    template<typename T1>
    ostream& operator<<(ostream& os, const Box<T1>& box){
        os << "Box(" << box.m_Range << "," << box.size() << box.size() << ")";
        return os;
    }
}