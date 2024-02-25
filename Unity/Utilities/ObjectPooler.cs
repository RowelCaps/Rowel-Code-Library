using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace ObjectPoolerSystem
{
    [System.Serializable]
    public class PooledData
    {
        public List<GameObject> spawned;
        public List<GameObject> pooled;

        public PooledData()
        {
            pooled = new List<GameObject>();
            spawned = new List<GameObject>();
        }
    }

    public class ObjectPooler : Singleton<ObjectPooler>
    {
        [SerializeField] private ObjectPooledDataDictionary m_objectPooled = new ObjectPooledDataDictionary();

        public GameObject SpawnObject(GameObject prefab, Transform parent = null)
        {
            if (prefab == null)
                return null;

            if(!m_objectPooled.ContainsKey(prefab))
            {
                PooledData pooledData = new PooledData();
                m_objectPooled.Add(prefab, pooledData);
            }

            PooledData data = m_objectPooled[prefab];

            GameObject spawn;
            
            if(data.pooled.Count > 0)
            {
                spawn = data.pooled[0];
                spawn.transform.parent = null;
                spawn.SetActive(true);

                data.pooled.Remove(spawn);
            }
            else
            {
                spawn = Instantiate(prefab, parent);

                PooledObject pooledObject =spawn.AddComponent<PooledObject>();
                pooledObject.PrefabOrigin = prefab;
            }

            data.spawned.Add(spawn);
            return spawn;
        }

        public void PoolObject(GameObject obj)
        {
            if (obj == null)
                return;

            PooledObject pooledObject = obj.GetComponent<PooledObject>();

            if (pooledObject == null)
                return;

            PooledData data = m_objectPooled[pooledObject.PrefabOrigin];
            
            data.spawned.Remove(obj);
            data.pooled.Add(obj);

            obj.transform.parent = transform;       
            obj.gameObject.SetActive(false);
        }
    }
}
