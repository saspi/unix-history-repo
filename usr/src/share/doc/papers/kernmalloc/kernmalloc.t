.\"	@(#)kernmalloc.t	1.1	(Copyright 1988 M. K. McKusick)	88/02/19
.\" reference a system routine name
.de RN
\fI\\$1\fP\^()\\$2
..
.\" reference a header name
.de H
.NH \\$1
\\$2
..
.EQ
delim $$
.EN
.rm CM
.TL
Design of a General Purpose Memory Allocator for the 4.3BSD UNIX\(dg Kernel
.FS
\(dgUNIX is a registered trademark of AT&T in the US and other countries.
.FE
.AU
Marshall Kirk McKusick
.AU
Michael J. Karels
.AI
Computer Systems Research Group
Computer Science Division
Department of Electrical Engineering and Computer Science
University of California, Berkeley
Berkeley, California  94720
.AB
This paper describes the current haphazard methods for allocating memory in
the 4.3BSD UNIX kernel.
Next it describes the requirements for a general purpose memory
allocator suitable for use in the kernel.
After describing the currently available memory allocators,
the paper describes how a hybrid of them are used to
implement a memory allocator suitable for all dynamic memory allocation
in the kernel.
It concludes with a discussion of our experience in using
the new memory allocator,
and directions for future improvements.
.AE
.LP
.H 1 "Kernel Memory Allocation in 4.3BSD
.PP
The 4.3BSD kernel has at least ten different memory allocators.
Often the allocations are for small pieces of memory that are only
needed for the duration of a single system call.
In a user process such short-term
memory would be allocated on the runtime stack.
Because the kernel has a limited runtime stack,
it is not feasible to allocate even small blocks of memory on it.
Consequently, such memory must be allocated through a more dynamic mechanism.
For example,
when the system must translate a pathname,
it must allocate a one kilobye buffer to hold the name.
.PP
Other blocks of memory must be more persistent than a single system call,
so really have to be allocated from dynamic memory.
Examples include protocol control blocks that remain throughout
the duration of the network connection.
.PP
Demands for dynamic memory allocation in the kernel have increased
as more services have been added.
Each time a new type of memory allocation has been required,
a specialized memory allocation scheme has been written to handle it.
Often the new memory allocation scheme has been built on top
of an older allocator.
For example, the block device subsystem provides a crude form of
memory allocation through the allocation of empty buffers.
The allocation is slow because of the implied semantics of
finding the oldest buffer, pushing its contents to disk if they are dirty,
and moving physical memory into or out of the buffer to create 
the requested size [Thompson78].
To reduce the overhead a ``new'' memory allocator is built that
allocates a pool of empty buffers.
It keeps them on a free list so they can
be quickly allocated and freed [McKusick85].
.PP
This memory allocation method has several drawbacks.
First, the new allocator can only handle a limited range of sizes.
Second, it depletes the buffer pool as it steals memory intended
to buffer disk blocks to other purposes.
Finally, it creates yet another interface of
which the programmer must be aware.
.PP
A generalized memory allocator is needed to reduce the complexity
of writing code inside the kernel.
Rather than providing many semi-specialized ways of allocating memory,
the kernel should provide a single general purpose allocator.
With only a single interface, 
programmers need not concern themselves with trying to figure
out the most appropriate way to allocate memory.
If a good general purpose allocator is available,
it helps avoid the syndrome of creating yet another special
purpose allocator.
.PP
To ease the task of understanding how to use the memory allocator,
it should have an interface similar to the interface
provided by the well known memory allocator provided for
applications programmers through the C library routines
.RN malloc
and
.RN free .
Like the C library interface,
the allocation routine should take a parameter specifying the
size of memory needed whose range  should not be constrained.
The free routine should take a pointer to the storage being freed,
and should not require additional information such as the size
of the piece of memory being freed.
.H 1 "Criterion for a Kernel Memory Allocator
.PP
The design specification for a kernel memory allocator is similar to,
but not identical with,
the design criterion for a user level memory allocator.
The first criterion for a memory allocator is that it make good use
of the physical memory.
Good use of memory is measured by the amount of memory needed to hold
a set of allocations at any point in time.
Percentage utilization is expressed as:
.EQ
utilization = used over allocated
.EN
Here, ``used'' is the sum of the memory that has been requested
and ``allocated'' is the amount of memory that has been
allocated to the pool from which the requests are filled.
A perfect memory allocator would have a utilization of 100%.
In practice,
having a 50% utilization is considered good [Korn85].
.PP
Good memory utilization in the kernel is more important than
in user processes.
Because user processes run in virtual memory,
unused parts of their address space can be paged out.
Thus pages that are part of the ``allocated'' pool that are not
being ``used'' need not tie up physical memory.
Because the kernel is not paged,
all pages in the ``allocated'' pool are held by the kernel and
cannot be used for other purposes.
Consequently, it is desirable to keep the utilization percentage
as high as possible.
It is also desirable to release memory from the ``allocated'' pool
rather than to hold them as is typically done with user processes.
.PP
The most important criterion for a memory allocator is that it be fast.
Memory allocation is done frequently,
so a slow memory allocator will degrade the system performance.
The kernel is more critical than user processes in this respect
because the kernel must allocate many data structure that user
programs would be able to (cheaply) allocate on their runtime stack.
In addition, the kernel represents the platform on which all user
processes run,
so if it is slow, it will degrade the performance of every process
that is running.
.PP
Another problem with a slow memory allocator is that programmers
of frequently used kernel interfaces will feel that they
cannot afford to use it as their primary memory allocator. 
Instead they will build their own memory allocator on top of the
original by maintaining their own pool of memory blocks.
Creating such a specialized memory allocator has the 
problem of creating multiple interfaces of which
the programmer must be aware.
.PP
Multiple allocators also reduces the efficiency with which memory is used.
The kernel ends up with many different free lists of memory
instead of a single free list from which all allocation can be drawn.
For example,
consider the case of two subsystem that need memory.
If they have their own free lists,
the amount of memory tied up in the two lists will be the
sum of the greatest amount of memory that each of
the two subsystems ever used.
If they share a freelist,
the amount of memory tied up in the free list may be as low as the
greatest amount of memory that either subsystem used.
As the number of subsystems grows,
the savings from having a single freelist grows.
.H 1 "Existing Implementations
.PP
There are many different algorithms and
implementations of memory allocators.
A survey of those available on UNIX systems appeared in [Korn85].
Nearly all the memory allocators tested made good used of memory, 
though most of them were too slow for use in the kernel.
The fastest memory allocator by nearly a factor of two,
was the memory allocator provided on 4.2BSD.
Unfortunately,
the 4.2BSD memory allocator also wasted twice as much memory
as its nearest competitor.
.PP
The 4.2BSD memory allocator works by maintaining a set of lists
that are ordered by powers of two.
Each list contains a set of memory blocks of its corresponding size.
To fulfill a memory request, 
the size of the request is rounded up to the next power of two.
A piece of memory is then removed from the list corresponding
to the specified power of two and returned to the requester.
Thus, a request for a block of memory of size 53 returns
a block from the 64-sized list.
A typical memory allocation requires a roundup calculation
followed by a linked list removal.
Only if the list is empty is a real memory allocation done.
The free operation is also fast;
the block of memory is put back onto the list from which it came.
.H 1 "Problems and Solutions Unique to a Kernel Allocator
.PP
There are several special conditions that arise when writing a
memory allocator for the kernel that do not apply to a user process
memory allocator.
First, the maximum memory allocation can be determined at 
the time that the machine is booted.
This number is never more than the amount of physical memory on the machine,
and is typically much less since a machine with all its
memory dedicated to the operating system is uninteresting to run on.
Thus the kernel can statically allocate a set of data structures
to manage its dynamically allocated memory.
These data structures never need to be grown to accommodate
memory requests, yet if properly designed need not be large.
.PP
For a user process, the maximum amount of memory that may be allocated
is a function of the maximum size of its virtual memory.
Although it could allocate static data structures to manage
its entire virtual memory,
even if they were efficiently encoded they would be potentially huge.
The other alternative is to allocate data structures as they are needed,
however that adds extra complications such as what to do if it cannot
allocate space for additional structures and how to link them all together.
.PP
Another special condition of the kernel memory allocator is that it
can control its own address space.
Unlike user processes that can only grow and shrink their heap at one end,
the kernel can keep an arena of kernel addresses and allocate
pieces from that arena which it then populates with physical memory.
The effect is much the same as a user process that has parts of
its address space paged out when they are not in use,
except that the kernel can explicitly control the set of pages
allocated to its address space.
The result is that the ``working set'' of pages in use by the
kernel exactly corresponds to the set of pages that it is really using.
.PP
A final special condition that applies to the kernel is that
all the different uses of dynamic memory are known in advance.
Each one of these uses of dynamic memory can be assigned a type.
For each type of dynamic memory that is allocated,
the kernel can provide allocation limits.
One reason given for having separate allocators is that
no single allocator could starve the rest of the kernel of all
its available memory.
By putting limits on each type of memory,
the single general purpose memory allocator can provide the same
protection against memory starvation\(dg.
.FS
\(dgOne might seriously ask the question what good it is if ``only''
one subsystem within the kernel hangs if it is something like the
network on a diskless workstation.
.FE
.PP
In profiling studies of the kernel, we found that most of its
allocations are for small objects.
Large allocations occur infrequently, 
and are typically for long lived objects
such as buffers to hold the superblock for
a file system when it is mounted.
Thus a memory allocator only needs to be fast for small pieces of memory.
.H 1 "Implementation of the Kernel Memory Allocator
.PP
In reviewing the available memory allocators,
none of their strategies could be used without some modification.
The kernel memory allocator that we ended up with is a hybrid
of the fast memory allocator found in the 4.2BSD C library
and a slower but more memory efficient first-fit allocator.
.PP
For maximum speed,
small allocations are done using the 4.2BSD power of two list strategy.
To avoid the cost of a subroutine call,
macros are provided to compute the list to use and remove an
element if it is available.
Only if the request cannot be fulfilled from a list is a call
made into the allocator itself.
To insure that the allocator is always called for large requests,
the lists corresponding to large allocations are always empty.
.PP
Similarly, freeing a block of memory can be done with a macro.
The macro computes the list on which to place the request
and puts it there.
The free routine is called only if the block of memory is
large enough to be a page-based allocation.
Including the cost of blocking out interrupts,
the allocation and freeing macros generate respectively
only nine and sixteen (simple) VAX instructions.
.PP
Because of the inefficiency of power of two allocation strategies
for large allocations,
a different strategy is used for allocations larger than two kilobytes.
The selection of two kilobytes is derived from our statistics on
the utilization of memory within the kernel,
that showed that 95 to 98% of allocations are of size one kilobyte or less.
Additionally the allocation method for large blocks is based on allocating
pieces of memory in multiples of pages.
Consequently the density of allocation for requests of size
$2~times~pagesize$ or less are identical.
On the VAX, the (software) page size is one kilobyte,
so two kilobytes is the smallest logical cutoff.
.PP
Large allocations are first rounded up to be a multiple of the page size.
The allocator then does a first-fit for the allocation from the
kernel address arena set aside for dynamic allocations.
Thus a request for a five kilobyte piece of memory will use exactly
five pages of memory rather than eight kilobytes as the power of two
allocation strategy would use.
.PP
Another technique to both improve the efficiency of memory utilization
and to speed allocation,
is to cluster same sized small allocations on a page.
When a list for a power of two allocation is empty,
a new page is allocated and entirely broken up into pieces of
the needed size.
This speeds future allocations since several pieces of memory
become available as a result of the call into the allocator.
.PP
Same size allocation per page improves the memory efficiency.
Instead of storing the size of each piece of memory with the piece itself,
the size information is associated with the memory page.
To determine the size of a piece of memory that is being freed,
the kernel calculates the page in which it resides,
and looks up the size associated with that page.
Eliminating the cost of the overhead per piece improved utilization
far more than expected.
The reason is that many allocations in the kernel are for blocks of 
memory that are a power of two in size.
Previously these requests had been nearly doubled to make room for
the size information.
Now they can be accommodated with no wasted memory.
.H 1 "Results of the Implementation
.PP
The new memory allocator was written about a year ago.
Conversion from the old memory allocators to the new allocator
have been going on ever since.
Many of the special purpose allocators have been eliminated.
This list includes
.RN calloc ,
.RN wmemall ,
and 
.RN zmemall .
Many of the special purpose memory allocators built on
top of other allocators have also been eliminated.
For example, the allocator that was built on top of the buffer pool allocator
.RN geteblk
to allocate pathname buffers in 
.RN namei
has been eliminated.
Because the typical allocation is so fast,
we have found that none of the special purpose pools are needed.
Indeed, the allocation is about the same as the cost of
allocating buffers from the network pool (``mbufs'').
Consequently applications that used to allocate network
buffers for their own uses can be switched over to using
the general purpose allocator without loss in performance.
.PP
The other major success has been in keeping the size information
on a per page basis.
This technique allowed the most frequently requested sizes to be
allocated without wastage.
It also reduced the amount of bookkeeping information associated
with the allocator to a compact four kilobytes of information
per megabyte of memory under management (with a one kilobyte page size).
.H 1 "Future Work
.PP
Our next immediate project is to convert many of the static
kernel tables to being dynamically allocated.
Static tables include the process table, the file table, the inodes table,
and the mount table.
Making these tables dynamic will have two benefits.
First, it will reduce the amount of memory
that must be statically allocated at boot time.
Second it will eliminate the arbitrary upper limit imposed
by the current static sizing.
Other researchers have already shown the memory savings
achieved by this conversion [Rodriguez88].
.PP
Under the current implementation,
memory is never moved from one sized list to another.
With the 4.2BSD memory allocator this causes problems
particularly for large allocations where a process may use
a quarter megabyte piece of memory once,
that is then never available for any other size request.
In our hybrid scheme,
memory can be shuffled between large requests so large blocks
of memory are never stranded as they are with the 4.2BSD allocator.
However pages allocated to small requests are allocated once
to a particular size and never changed thereafter.
If a burst of requests came in for a particular size,
that particular size would acquire a large amount of memory
that would then not be available for other future requests.
In practice, we do not find that such behavior occurs,
however we have been investigating ways to handles such problems
if they become problematical in the future.
.PP
Our current investigations have involved having a routine
that can run as part of the idle loop that would sort the elements
on each of the free lists into order of increasing address.
Since any given page has only one size of elements allocated from it,
the effect of the sorting would be to sort the list into distinct pages.
When all the pieces of a page became free,
the page itself could be released back to the free pool so that 
it could be allocated to another purpose.
Although there is no guarantee that all the pieces of a page would ever
be freed,
most allocations are short-lived lasting only the duration of
an open file descriptor, an open network connection, or a system call.
Since new allocations would be concentrated from the page sorted to
the front of the list,
returns of elements from pages at the back would eventually
allow pages later in the list to be freed.
.H 1 "Acknowledgments
.PP
In the spirit of community support,
we have made various versions of our allocator available to our test sites.
They have been busily burning it in and giving
us feedback on their experiences.
We acknowledge their invaluable input.
.H 1 "References
.sp
.IP Korn85 \w'Rodriguez88\0\0'u
David Korn, Kiem-Phong Vo,
``In Search of a Better Malloc''
\fIProceedings of the Portland Usenix Conference\fP,
pp 489-506, June 1985.
.sp
.IP McKusick85
M. McKusick, M. Karels, S.Leffler,
``Performance Improvements and Functional Enhancements in 4.3BSD''
\fIProceedings of the Portland Usenix Conference\fP,
pp 519-531, June 1985.
.sp
.IP Rodriguez88
Robert Rodriguez, Matt Koehler, Larry Palmer, Ricky Palmer,
``A Dynamic UNIX Operating System''
Digital Equipment Corporation, ULTRIX Engineering, Nashua, NH 03062,
To appear.
.IP Thompson78
Ken Thompson,
``UNIX Implementation''
\fIBell System Technical Journal\fP, volume 57, number 6,
pp 1931-1946, 1978.
