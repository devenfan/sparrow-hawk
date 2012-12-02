#ifndef W1_USERSERVICE_H_INCLUDED
#define W1_USERSERVICE_H_INCLUDED

/**
 * Deven # 2012-11-03:
 * 1. To support multi-masters, interface has been changed.
 *
 * Deven # 2012-11-02:
 * 1. Multi-masters are supported from kernel.
 *
*/


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Callback utility for acquiring the w1 wakelock.
 * This can be used to prevent the CPU from suspending while handling w1 events.
 */
typedef void (* w1_acquire_wakelock)();


/*
 * Callback utility for releasing the w1 wakelock.
 */
typedef void (* w1_release_wakelock)();


/*
 * Callback for creating a thread that can call into the Java framework code.
 * This must be used to create any threads that report events up to the android framework.
 */
typedef pthread_t (* w1_create_thread)(const char* name, void (*start)(void *), void* arg);



typedef void w1_master_added(w1_master_id master_id);

typedef void w1_master_removed(w1_master_id master_id);


typedef void w1_slave_added(w1_master_id master_id, w1_slave_rn salve_rn);

typedef void w1_slave_removed(w1_master_id master_id, w1_slave_rn salve_rn);



typedef struct w1_user_callbacks{

    w1_acquire_wakelock acquire_wakelock_cb;

    w1_release_wakelock release_wakelock_cb;

    w1_create_thread create_thread_cb;

    w1_master_added * master_added_callback;

    w1_master_removed * master_removed_callback;

    w1_slave_added * slave_added_callback;

    w1_slave_removed * slave_removed_callback;

}w1_user_callbacks;


typedef struct w1_user_service {

    void (*init)(w1_user_callbacks * callbacks);

    BOOL (*start)();

    void (*stop)();

    BOOL (*begin_exclusive)();  //exclusive for all the w1 masters

    void (*end_exclusive)();    //exclusive for all the w1 masters

    //BOOL (*begin_exclusive)(w1_master_id masterId);   //exclusive for one w1 master

    //void (*end_exclusive)(w1_master_id masterId); //exclusive for one w1 master

    //w1_master_id (*get_current_master)();	//support only one w1 master

    //void (*get_current_slaves)(w1_slave_rn * slaveIDs, int * slaveCount);	//support only one w1 master

    BOOL (*list_masters)(w1_master_id * masterIDs, int * masterCount);

    BOOL (*search_slaves)(w1_master_id masterId, w1_slave_rn * slaves, int * pSlaveCount);

    BOOL (*master_reset)(w1_master_id masterId);

    BOOL (*master_read)(w1_master_id masterId, int readLen, void * data);

    BOOL (*master_write)(w1_master_id masterId, int writeLen, void * data);

    BOOL (*master_touch)(w1_master_id masterId, void * dataIn, int dataInLen, void * dataOut, int * pDataOutLen);


} w1_user_service;



#ifdef __cplusplus
}
#endif


#endif // W1_USERSERVICE_H_INCLUDED
