using System;
using System.Collections;
using System.Collections.Generic;
using Unity.Mathematics;
using Unity.VisualScripting;
using UnityEngine;

public class VertexSnapController : MonoBehaviour
{
    [SerializeField] MeshController m_meshController;
    [SerializeField] GameObject spawnObjectPrefab;
    [SerializeField] Transform m_spawnPoint;
    [SerializeField] BoxCollider m_collider;
    [SerializeField] float minSpawnTime = 0.3f;
    [SerializeField] float maxSpawnTime = 0.3f;
    [SerializeField] int spawnCount = 20;

    List<VertexSnapObject> m_snapObjects = new List<VertexSnapObject>();
    List<VertexSnapObject> m_idleSnapObject = new List<VertexSnapObject>();

    private void Start()
    {
        StartCoroutine(SpawnObjects());
    }

    IEnumerator SpawnObjects()
    {
        for(int i = 0; i < spawnCount; i++)
        {
            float spawnInterval = UnityEngine.Random.Range(minSpawnTime, maxSpawnTime);
            yield return new WaitForSeconds(spawnInterval);

            GameObject snapObj = GameObject.Instantiate(spawnObjectPrefab, this.transform);
            Bounds bounds = m_collider.bounds;

            Vector3 randomPointInBoundingBox = new Vector3(
                UnityEngine.Random.Range(bounds.min.x, bounds.max.x),
                UnityEngine.Random.Range(bounds.min.y, bounds.max.y),
                UnityEngine.Random.Range(bounds.min.z, bounds.max.z));

            snapObj.transform.position = randomPointInBoundingBox;

            if(snapObj.TryGetComponent<VertexSnapObject>(out VertexSnapObject obj))
            {
                RegisterSnapObject(obj);
            }
        }
    }

    public void RegisterSnapObject(VertexSnapObject snapObject)
    {
        if (m_snapObjects.Contains(snapObject))
            return;

        snapObject.OnStoppedMovement += OnSnapObjectStopped;
        m_snapObjects.Add(snapObject);
    }

    void OnSnapObjectStopped(VertexSnapObject snapObject)
    {
        if (m_idleSnapObject.Contains(snapObject))
            return;

        m_idleSnapObject.Add(snapObject);

        if(m_snapObjects.Count == m_idleSnapObject.Count)
        {
            SnapMeshToObjects();
        }
    }

    public void SnapMeshToObjects()
    {
        for(int i = 0; i  < m_idleSnapObject.Count; i++)
        {
            int lowestIndex = i;
            float lowestPointY = m_idleSnapObject[i].GetLowestPointMesh().y;

            for(int j = 0; j < m_idleSnapObject.Count; j++)
            {
                if (j == i)
                    continue;

                float jLowestPointY = m_idleSnapObject[j].GetLowestPointMesh().y;

                if(jLowestPointY < lowestPointY)
                {
                    lowestIndex = j;
                }
            }

            VertexSnapObject tempObject = m_idleSnapObject[i];
            m_idleSnapObject[i] = m_idleSnapObject[lowestIndex];
            m_idleSnapObject[lowestIndex] = tempObject;
        }

        m_idleSnapObject.ForEach(x =>
        {
            Vector2 position = x.transform.position;
            position.y = x.GetLowestPointMesh().y;

            Debug.Log(x.GetHighestPointMesh());
            m_meshController.VertexSnapToPosition(x.GetHighestPointMesh(), position.y);
        });

        m_meshController.RecalculateMesh();

        m_snapObjects.Clear();
        //For testing only
        for (int i =0; i < m_idleSnapObject.Count; i++)
        {
            GameObject obj = m_idleSnapObject[i].gameObject;
            m_idleSnapObject[i] = null;

            Debug.Log(obj);
            Destroy(obj);
        }

        m_idleSnapObject.Clear();
    }
}
