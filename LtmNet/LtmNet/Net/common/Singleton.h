#ifndef __LTM_NET_SINGLETON__
#define __LTM_NET_SINGLETON__

/*************************		单实例基类			*************************/

namespace ltm_net
{

template< typename T >
class Singleton
{
    
public:
    static T* GetInstance()
    {
      
        if ( _pInstance == NULL )
        {
            _pInstance = new T();
        }
        
        return _pInstance;
    };
    
protected:
    Singleton(){};
    virtual ~Singleton(){ if( _pInstance != NULL ){ delete _pInstance; };  _pInstance=NULL; };
    
    static T* _pInstance;
    


};

template< typename T > T* MSingleton<T>::_pInstance = NULL;

};

#endif