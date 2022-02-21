//
// Created by dewe on 1/9/22.
//

#ifndef SAM_RL_UPDATEABLE_H
#define SAM_RL_UPDATEABLE_H


namespace gym {
    template<class ... Args>
    struct Updateable {
    public:
        virtual void update(Args && ...) = 0;
    };

    template<class ... Args>
    struct Updater {
    private:
        std::vector<Updateable<Args ...>*> updateables{};
    public:

        void subscribe(Updateable<Args ...>* sub){
            assert(sub);
            updateables.emplace_back(sub);
        }

        void notifyOne(Args && ... args){
            if (not updateables.empty())
                updateables[0]->update( std::move(args ...));
        }
    };
}
#endif //SAM_RL_UPDATEABLE_H
