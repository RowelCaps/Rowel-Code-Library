using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Rigidbody))]
[RequireComponent(typeof(MeshFilter))]
[RequireComponent(typeof(MeshRenderer))]
public class VertexSnapObject : MonoBehaviour
{
    [SerializeField] private float m_movementThreshold = 0.1f;
    [SerializeField] private float m_delayCheck = 1;
    [SerializeField] private float m_onStopDelayInvokation = 1;

    private bool m_checkMovement = false;
    private bool m_done = false;
    private float timer = 0;

    private MeshFilter m_meshFilter;
    private Rigidbody m_rb;
    private Collider m_collider;

    public System.Action<VertexSnapObject> OnStoppedMovement;

    private void Awake()
    {
        m_meshFilter = GetComponent<MeshFilter>();
        m_rb = GetComponent<Rigidbody>();
        m_collider = GetComponent<Collider>();
    }

    private void OnEnable()
    {
        StartCoroutine(DelayVelocityCheck());
    }

    private void Update()
    {
        if (!m_checkMovement)
            return;

        Vector3 velocity = m_rb.velocity;

        if (velocity.magnitude < m_movementThreshold)
        {
            if (OnStoppedMovement != null && timer >= m_onStopDelayInvokation)
            {
                OnStoppedMovement.Invoke(this);
                m_checkMovement = false;
                m_done = true;
                timer = 0;
            }
            else
                timer += Time.deltaTime;
        }
        else
            timer = 0;
    }

    IEnumerator DelayVelocityCheck()
    {
        yield return new WaitForSeconds(m_delayCheck);

        m_checkMovement = true;
    }

    public Vector3 GetHighestPointMesh()
    {
        Bounds bounds = m_collider.bounds;
        return bounds.center + Vector3.up * bounds.extents.y;
    }

    public Vector3 GetLowestPointMesh()
    {
        Bounds bounds = m_collider.bounds;
        return bounds.center - Vector3.up * bounds.extents.y;

    }
}
