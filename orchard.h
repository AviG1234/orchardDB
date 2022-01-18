#ifndef ORCHARD_H
#define ORCHARD_H
#include <list>
#include <set>
#include <iterator>
#include <utility>

#include <iostream>
using namespace std;

struct tree_list;
struct Fruit //basic fruit structure.
{
    int fruitID;
    int ripeRate;
    tree_list* tree;
    Fruit(int ID = 0, int ripe = 0) : fruitID(ID), ripeRate(ripe){}
};

struct CompByID //structure to compare fruits by id, for sorting in set of fruit pointer objects.
{
    bool operator () (Fruit* a, Fruit* b) const {return a->fruitID < b->fruitID;}
};

struct CompByRipeRate //structure to compare fruits by ripeRate and id, for sorting in list of fruit pointer objects.
{
    bool operator () (Fruit* a, Fruit* b)
    {
        if(a->ripeRate < b->ripeRate) return true;
        if(a->ripeRate == b->ripeRate && a->fruitID < b->fruitID) return true;
        return false;
    }
};

struct tree_list //structure of tree containing and managing a sorted list of fruits.
{
    std::list<Fruit*> list;
    std::list<Fruit*>::iterator IT;
    CompByRipeRate Comp;
    int tree;
    void AddFruit(Fruit* new_fruit) {
        for(IT = list.begin(); IT != list.end(); ++IT){
            if(Comp(new_fruit, *IT)){
                list.insert(IT,new_fruit);
                return;
            }
        }
        list.insert(IT, new_fruit);
    }
    void PickFruit(int ID){
        for(IT = list.begin(); IT != list.end(); ++IT){
            if((*IT)->fruitID == ID){
                delete *IT;
                list.erase(IT);
                return;
            }
        }
    }
    void RateFruit(int fruitID, int ripeRate){
        Fruit* new_fruit;
        for(IT = list.begin(); IT != list.end(); ++IT){
            if((*IT)->fruitID == fruitID){
                new_fruit = (*IT);
                break;
            }
        }
        new_fruit->ripeRate = ripeRate;
        list.erase(IT);
        AddFruit(new_fruit);
    }
    void GetAllFruitsByRate(int **fruits, int *numOfFruits){
        *numOfFruits = list.size();
        *fruits = NULL;
        if(*numOfFruits == 0){
            return;
        }
        try{
            auto arr = new int [*numOfFruits];
            int count = 0;
            for(auto it : list){
                arr[count] = it->fruitID;
                ++count;
            }
            *fruits = arr;
        }
        catch(std::bad_alloc&){
            throw -2;//ALLOCATION_ERROR
            return;
        }
    }

    void UpdateRottenFruits(int rottenBase, int rottenFactor){
        std::list<Fruit*> temp;
        for(IT = list.begin(); IT != list.end();){
            if(((*IT)->fruitID)%rottenBase == 0){
                Fruit* fruit = *IT;
                list.erase(IT++);
                fruit->ripeRate = (fruit->ripeRate)*rottenFactor;
                temp.push_back((fruit));
            }
            else{
                ++IT;
            }
        }
        list.merge(temp, Comp);
    }

    tree_list(int x = 0) : tree(x){}
};

struct CompTree_List //structure to compare tree_lists(trees) by index number (i*N+j), for sorting in set of pointer tree_list objects.
{
    bool operator () (tree_list* a, tree_list* b) const {return a->tree < b->tree;}
};

//main DS containing and managing a set of all fruits sorted by fruitID and a set of tree_list sorted by tree index
//were fruits of the tree are sorted by ripeRate.
class Orchard
{
private:
    std::set<Fruit*,CompByID> set_ID;
    std::set<tree_list*,CompTree_List> set_tree;
    int num_of_trees;
    Fruit tempID;
    tree_list tempTree;

public:
    int index(int i, int j){return i*num_of_trees + j;}
    bool bad_index(int i, int j ){return (i<0 || j<0 || i>= num_of_trees || j>= num_of_trees);}
    void PlantTree(int i, int j){
        if(bad_index(i,j)){
            throw -3;//INVALID_INPUT - if i,j are not in [0,N-1]
            return;
        }
        tree_list* new_tree = new tree_list(index(i,j));
        auto pair = set_tree.insert(new_tree);
        if(!pair.second){throw -1;}//FAILURE - If a tree already exists in the spot
        return;
    }
    void AddFruit(int i, int j, int fruitID, int ripeRate){
        if(bad_index(i,j)){
            throw -3;//INVALID_INPUT - if i,j are not in [0,N-1]
            return;
        }
        if(fruitID <= 0 || ripeRate <= 0){
            throw -3;//INVALID_INPUT fruitID<=0 or ripeRate<=0
            return;
        }
        Fruit* new_ptr;
        try{new_ptr = new Fruit(fruitID, ripeRate);}
        catch(std::bad_alloc&){
            throw -2;//ALLOCATION_ERROR
            return;
        }
       tree_list temp_list(index(i,j));
        auto treeIT = set_tree.find(&temp_list);
        if(treeIT == set_tree.end()){
            throw -1;//FAILURE -(i,j)'th tree doesn't exist
            return;
        }
        auto pair = set_ID.insert(new_ptr);
        if(!pair.second){
            throw -1;//FAILURE -	If fruitID is already used
            return;
        }
        new_ptr->tree = *treeIT;
        (*treeIT)->AddFruit(new_ptr);
    }
    void PickFruit(int fruitID){
        if(fruitID <= 0 ){
            throw -3;//INVALID_INPUT
            return;
        }
        tempID.fruitID = fruitID;
        auto ITset = set_ID.find(&tempID);
        if(ITset == set_ID.end()){
            throw -1;//FAILURE -	If fruitID doesn't exist
            return;
        }
        (*ITset)->tree->PickFruit(fruitID);
        set_ID.erase(ITset);
    }
    void RateFruit(int fruitID, int ripeRate){
        if(fruitID <= 0  || ripeRate <= 0){
            throw -3;//INVALID_INPUT
            return;
        }
        tempID.fruitID = fruitID;
        auto ITset = set_ID.find(&tempID);
        if(ITset == set_ID.end()){
            throw -1;//FAILURE -	If fruitID doesn't exist
            return;
        }
        (*ITset)->tree->RateFruit(fruitID,ripeRate);
    }
    void GetBestFruit(int i, int j, int *fruitID){
        if(bad_index(i,j)){
            throw -3;//INVALID_INPUT - if i,j are not in [0,N-1]
            return;
        }
        tempTree.tree = index(i,j);
        auto treeIT = set_tree.find(&tempTree);
        if(treeIT == set_tree.end()){
            throw -1;//FAILURE - If the (i,j)'th tree doesn't exist
            return;
        }
        if((*treeIT)->list.empty()){
            *fruitID = -1;
            return;
        }
        *fruitID = (*((*treeIT)->list.begin()))->fruitID;
    }

    void GetAllFruitsByRate(int i, int j, int **fruits, int *numOfFruits){
       if(bad_index(i,j)){
            throw -3;//INVALID_INPUT - if i,j are not in [0,N-1]
            return;
        }
        tempTree.tree = index(i,j);
        auto treeIT = set_tree.find(&tempTree);
        if(treeIT == set_tree.end()){
            throw -1;//FAILURE - If the (i,j)'th tree doesn't exist
            return;
        }
        (*treeIT)->GetAllFruitsByRate(fruits,numOfFruits);
    }
    void UpdateRottenFruits(int rottenBase, int rottenFactor){
        int T=0;
        for(auto tree: set_tree){
            tree->UpdateRottenFruits(rottenBase, rottenFactor);
        }
    }

    Orchard(int N) : num_of_trees(N) {}
    ~Orchard(){
        for(auto IT = set_tree.begin(); IT != set_tree.end();++IT){
            delete *(IT);
            set_tree.erase(IT++);
        }
        for(auto IT = set_ID.begin(); IT != set_ID.end();++IT){
            delete *(IT);
            set_ID.erase(IT++);
        }
    }
};

#endif // ORCHARD_H
