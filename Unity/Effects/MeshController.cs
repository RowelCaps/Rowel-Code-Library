using System.Collections;
using System.Collections.Generic;
using UnityEditor.Experimental.GraphView;
using UnityEditor.ShaderGraph.Internal;
using UnityEngine;
using UnityEngine.Rendering;

public class MeshController : MonoBehaviour
{
    [Header("UV Recalculation")]
    [SerializeField] float m_recalculationScale = 0.1f;
    [SerializeField] Vector2 m_uvOffset = new Vector2(0.5f, 0.5f);

    [Header("Mesh Displacement Settings")]
    [SerializeField] float m_maxHeight = 4;
    [SerializeField] float m_minHeight = -3;

    [Header("Mesh Pushing Settings:")]
    [SerializeField] float m_deformationStrenght = 2;
    [SerializeField] float m_smoothingFactor = 2;
    [SerializeField] float m_forceRadius = 10.0f;

    [Header("Mesh Digging Settings")]
    [SerializeField] float m_digStrenght = 0.5f;
    [SerializeField] float m_digInfluenceRadius = 2;

    [Header("Vertex Snapping Settings")]
    [SerializeField] float m_snapInfluenceRadius = 1.5f;
    [SerializeField] float m_snapInfluenceStrenght = 1;



    private Mesh m_mesh;
    private MeshFilter m_meshFilter;
    private MeshCollider m_meshCollider;
    private Vector3[] vertices, modifiedVerts;

    // Start is called before the first frame update
    void Awake()
    {
        m_meshFilter = GetComponent<MeshFilter>();
        m_meshCollider = GetComponentInChildren<MeshCollider>();
        m_mesh = m_meshFilter.mesh;

        vertices = m_mesh.vertices;
        modifiedVerts = m_mesh.vertices;
    }

    // Update is called once per frame
    void Update()
    {    
    }

    public void RecalculateMesh()
    {
        m_mesh.vertices = modifiedVerts;
        m_meshCollider.sharedMesh = m_mesh;
        m_mesh.RecalculateNormals();
        RecalculateUV();
    }

    void RecalculateUV()
    {
        if (m_meshFilter != null)
        {
            int[] triangles = m_mesh.triangles;
            Vector2[] uvs = m_mesh.uv;

            for (int index = 0; index < triangles.Length; index += 3)
            {
                // Get the three vertices bounding this triangle.
                Vector3 v1 = vertices[triangles[index]];
                Vector3 v2 = vertices[triangles[index + 1]];
                Vector3 v3 = vertices[triangles[index + 2]];

                // Compute a vector perpendicular to the face.
                Vector3 normal = Vector3.Cross(v3 - v1, v2 - v1);

                // Form a rotation that points the z+ axis in this perpendicular direction.
                // Multiplying by the inverse will flatten the triangle into an xy plane.
                Quaternion rotation = Quaternion.Inverse(Quaternion.LookRotation(normal));

                // Assign the uvs, applying a scale factor to control the texture tiling.
                uvs[triangles[index]] = ((Vector2)(rotation * v1) * -m_recalculationScale) + m_uvOffset;
                uvs[triangles[index + 1]] = ((Vector2)(rotation * v2) * -m_recalculationScale) + m_uvOffset;
                uvs[triangles[index + 2]] = ((Vector2)(rotation * v3) * -m_recalculationScale) + m_uvOffset;
            }
            
            m_mesh.uv = uvs;
        }
    }

    public void PushVertices(Collider col, Vector3 direction)
    {
        Bounds colliderBounds = col.bounds;

        for(int i =0; modifiedVerts.Length > i; i++)
        {
            Vector3 vertWorldPos = transform.TransformPoint(modifiedVerts[i]);

            Vector3 closestPoint = colliderBounds.ClosestPoint(vertWorldPos);
            float distance = Vector3.Distance(vertWorldPos, closestPoint);

            if (distance > m_forceRadius)
                continue;

            float horizontalForce = m_deformationStrenght *  (1 - (distance / m_forceRadius));
            float verticalForce = m_deformationStrenght * (distance / m_forceRadius);

            Vector3 force = direction * (horizontalForce);

            force.y = 0;
            force = force + new Vector3(0, verticalForce, 0);

            modifiedVerts[i] = modifiedVerts[i] + ((force / m_smoothingFactor) * Time.deltaTime);
        }

        RecalculateMesh();
    }

    public void Dig(Collider col)
    {
        Bounds localBounds = col.bounds;

        // Transform the local min and max points to world space
        float bottom = localBounds.min.y;
        Debug.Log(col.gameObject.name);

        for (int i =0; i < modifiedVerts.Length; i++)
        {
            Vector3 vertWorldPos = transform.TransformPoint(modifiedVerts[i]);

            if (localBounds.Contains(vertWorldPos))
            {
                vertWorldPos.y = bottom;

                modifiedVerts[i] = transform.InverseTransformPoint(vertWorldPos);

                Vector3 closestPoint = localBounds.ClosestPoint(vertWorldPos);
                Vector3 direction = (closestPoint - modifiedVerts[i]).normalized;


                for (int j = 0; j < modifiedVerts.Length; j++)
                {
                    if (j == i)
                        continue;

                    Vector3 influenceVertWorldPos = transform.TransformPoint(modifiedVerts[j]);

                    float distance = Vector3.Distance(vertWorldPos, influenceVertWorldPos);

                    if(distance < m_digInfluenceRadius)
                    {
                        float influence = 1 - (distance / m_digStrenght);
                        modifiedVerts[j] += direction * influence * m_digStrenght;
                    }
                }
            }
        }

        RecalculateMesh();
    }

    public void VertexSnapToPosition(Vector3 position, float lowestYPos)
    {
        float smallestDistance = float.PositiveInfinity;
        int smallestIndex = -1;

        for(int i=0; i<modifiedVerts.Length;i++)
        {
            Vector3 vertWorldPos = transform.TransformPoint(modifiedVerts[i]);

            if (vertWorldPos.y > lowestYPos)
                continue;

            float distance = Vector3.Distance(vertWorldPos, position);

            if(distance < smallestDistance && distance < m_snapInfluenceRadius)
            {
                smallestIndex = i;
                smallestDistance = distance;
            }
        }

        if (smallestIndex < 0)
            return;

        Vector3 localPosition = transform.InverseTransformPoint(position);

        Debug.Log(smallestIndex + " " + modifiedVerts.Length);

        float prevY = modifiedVerts[smallestIndex].y;
        float distanceY = localPosition.y - prevY;

        modifiedVerts[smallestIndex].y = localPosition.y;

        for(int i =0; i<modifiedVerts.Length;i++)
        {
            if (i == smallestIndex || modifiedVerts[i].y >= modifiedVerts[smallestIndex].y)
                continue;

            float vertexDistance = modifiedVerts[smallestIndex].y - modifiedVerts[i].y;
            float worldDistance = Vector3.Distance(modifiedVerts[smallestIndex], modifiedVerts[i]);

            if (worldDistance >= m_snapInfluenceRadius)
                continue;

            float influence = (vertexDistance / m_snapInfluenceRadius);

            modifiedVerts[i].y += Mathf.Clamp(vertexDistance * influence, 0, vertexDistance);
        }
    }



    private Bounds TransformBoundsToWorldSpace(Bounds localBounds, Transform transform)
    {
        Matrix4x4 localToWorldMatrix = transform.localToWorldMatrix;
        return new Bounds(localToWorldMatrix.MultiplyPoint(localBounds.center), localBounds.size);
    }
}
