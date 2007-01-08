/*
* PROJECT:         ReactOS Kernel
* LICENSE:         GPL - See COPYING in the top level directory
* FILE:            ntoskrnl/include/ob_x.h
* PURPOSE:         Intenral Inlined Functions for the Object Manager
* PROGRAMMERS:     Alex Ionescu (alex.ionescu@reactos.org)
*/

#if DBG
VOID
FORCEINLINE
ObpCalloutStart(IN PKIRQL CalloutIrql)
{
    /* Save the callout IRQL */
    *CalloutIrql = KeGetCurrentIrql();
}

VOID
FORCEINLINE
ObpCalloutEnd(IN KIRQL CalloutIrql,
              IN PCHAR Procedure,
              IN POBJECT_TYPE ObjectType,
              IN PVOID Object)
{
    /* Detect IRQL change */
    if (CalloutIrql != KeGetCurrentIrql())
    {
        /* Print error */
        DbgPrint("OB: ObjectType: %wZ  Procedure: %s  Object: %08x\n",
                 &ObjectType->Name, Procedure, Object);
        DbgPrint("    Returned at %x IRQL, but was called at %x IRQL\n",
                 KeGetCurrentIrql(), CalloutIrql);
        DbgBreakPoint();
    }
}
#else
VOID
FORCEINLINE
ObpCalloutStart(IN PKIRQL CalloutIrql)
{
    /* No-op */
    UNREFERENCED_PARAMETER(CalloutIrql);
}

VOID
FORCEINLINE
ObpCalloutEnd(IN KIRQL CalloutIrql,
              IN PCHAR Procedure,
              IN POBJECT_TYPE ObjectType,
              IN PVOID Object)
{
    UNREFERENCED_PARAMETER(CalloutIrql);
}
#endif

VOID
FORCEINLINE
ObpEnterObjectTypeMutex(IN POBJECT_TYPE ObjectType)
{
    /* Sanity check */
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    /* Enter a critical region and acquire the resource */
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&ObjectType->Mutex, TRUE);
}

VOID
FORCEINLINE
ObpLeaveObjectTypeMutex(IN POBJECT_TYPE ObjectType)
{
    /* Enter a critical region and acquire the resource */
    ExReleaseResourceLite(&ObjectType->Mutex);
    KeLeaveCriticalRegion();

    /* Sanity check */
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
}

VOID
FORCEINLINE
ObpReleaseCapturedAttributes(IN POBJECT_CREATE_INFORMATION ObjectCreateInfo)
{
    /* Check if we have a security descriptor */
    if (ObjectCreateInfo->SecurityDescriptor)
    {
        /* Release it */
        SeReleaseSecurityDescriptor(ObjectCreateInfo->SecurityDescriptor,
                                    ObjectCreateInfo->ProbeMode,
                                    TRUE);
        ObjectCreateInfo->SecurityDescriptor = NULL;
    }
}

PVOID
FORCEINLINE
ObpAllocateCapturedAttributes(IN PP_NPAGED_LOOKASIDE_NUMBER Type)
{
    PVOID Buffer;
    PNPAGED_LOOKASIDE_LIST List;
    PKPRCB Prcb = KeGetCurrentPrcb();

    /* Get the P list first */
    List = (PNPAGED_LOOKASIDE_LIST)Prcb->PPLookasideList[Type].P;

    /* Attempt allocation */
    List->L.TotalAllocates++;
    Buffer = (PVOID)InterlockedPopEntrySList(&List->L.ListHead);
    if (!Buffer)
    {
        /* Let the balancer know that the P list failed */
        List->L.AllocateMisses++;

        /* Try the L List */
        List = (PNPAGED_LOOKASIDE_LIST)Prcb->PPLookasideList[Type].L;
        List->L.TotalAllocates++;
        Buffer = (PVOID)InterlockedPopEntrySList(&List->L.ListHead);
        if (!Buffer)
        {
            /* Let the balancer know the L list failed too */
            List->L.AllocateMisses++;

            /* Allocate it */
            Buffer = List->L.Allocate(List->L.Type, List->L.Size, List->L.Tag);
        }
    }

    /* Return buffer */
    return Buffer;
}

VOID
FORCEINLINE
ObpFreeCapturedAttributes(IN PVOID Buffer,
                          IN PP_NPAGED_LOOKASIDE_NUMBER Type)
{
    PNPAGED_LOOKASIDE_LIST List;
    PKPRCB Prcb = KeGetCurrentPrcb();

    /* Use the P List */
    List = (PNPAGED_LOOKASIDE_LIST)Prcb->PPLookasideList[Type].P;
    List->L.TotalFrees++;

    /* Check if the Free was within the Depth or not */
    if (ExQueryDepthSList(&List->L.ListHead) >= List->L.Depth)
    {
        /* Let the balancer know */
        List->L.FreeMisses++;

        /* Use the L List */
        List = (PNPAGED_LOOKASIDE_LIST)Prcb->PPLookasideList[Type].L;
        List->L.TotalFrees++;

        /* Check if the Free was within the Depth or not */
        if (ExQueryDepthSList(&List->L.ListHead) >= List->L.Depth)
        {
            /* All lists failed, use the pool */
            List->L.FreeMisses++;
            List->L.Free(Buffer);
        }
    }
    else
    {
        /* The free was within the Depth */
        InterlockedPushEntrySList(&List->L.ListHead,
                                  (PSINGLE_LIST_ENTRY)Buffer);
    }
}

VOID
FORCEINLINE
ObpFreeAndReleaseCapturedAttributes(IN POBJECT_CREATE_INFORMATION ObjectCreateInfo)
{
    /* First release the attributes, then free them from the lookaside list */
    ObpReleaseCapturedAttributes(ObjectCreateInfo);
    ObpFreeCapturedAttributes(ObjectCreateInfo, LookasideCreateInfoList);
}

