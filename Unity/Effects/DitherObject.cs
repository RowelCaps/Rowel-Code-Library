using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DitherObject : MonoBehaviour
{
    [Header("Dither Settings")]
    [SerializeField] private bool HideOnStart = false;
    [SerializeField] private float hideSpeed = 5;
    [SerializeField] private float hideOpacity = 0;
    [SerializeField] private float VisibleSpeed = 5;
    [SerializeField] private float VisibleOpacity = 1;

    List<Material> materials = new List<Material>();

    private void Awake()
    {
        foreach(MeshRenderer rend in GetComponentsInChildren<MeshRenderer>())
        {
            Material m = rend.material;
            m.SetFloat("_Opacity", HideOnStart ? hideOpacity : VisibleOpacity);

            materials.Add(m);
        }
    }

    public void SetToVisible(bool value)
    {
        StopAllCoroutines();

        if (value)
            StartCoroutine(InterpAlpha(VisibleOpacity, VisibleSpeed));
        else
            StartCoroutine(InterpAlpha(hideOpacity, hideSpeed));
    }

    IEnumerator InterpAlpha(float target, float speed)
    {
        List<MatInterpInfo> interpInfo = new List<MatInterpInfo>();

        foreach(Material m in materials)
        {
            MatInterpInfo info = new MatInterpInfo(m, m.GetFloat("_Opacity"));
            interpInfo.Add(info);
        }

        float t = 0;

        while(t < 1)
        {
            t += speed * Time.deltaTime;

            print(t);

            foreach(MatInterpInfo i in interpInfo)
            {
                float a = Mathf.Lerp(i.initial, target,t);
                i.mat.SetFloat("_Opacity", a); 
            }

            yield return null;
        }
    }

    class MatInterpInfo
    {
        public Material mat;
        public float initial;

        public MatInterpInfo(Material mat, float initial)
        {
            this.mat = mat;
            this.initial = initial;
        }
    }
}
