/*-
 * Copyright (c) 2007 Rui Paulo <rpaulo@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 *
 */

/*
 * Device driver for Intel's On Die thermal sensor via MSR.
 * First introduced in Intel's Core line of processors.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/module.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/sensors.h>
#include <sys/proc.h>	/* for curthread */
#include <sys/sched.h>

#include <machine/specialreg.h>
#include <machine/cpufunc.h>
#include <machine/md_var.h>

extern int smp_cpus;

struct coretemp_softc {
	struct ksensordev	sc_sensordev;
	struct ksensor		sc_sensor;
	device_t		sc_dev;
	int			sc_tjmax;
};

/*
 * Device methods.
 */
static void	coretemp_identify(driver_t *driver, device_t parent);
static int	coretemp_probe(device_t dev);
static int	coretemp_attach(device_t dev);
static int	coretemp_detach(device_t dev);

static int	coretemp_get_temp(device_t dev);
static void	coretemp_refresh(void *arg);

static device_method_t coretemp_methods[] = {
	/* Device interface */
	DEVMETHOD(device_identify,	coretemp_identify),
	DEVMETHOD(device_probe,		coretemp_probe),
	DEVMETHOD(device_attach,	coretemp_attach),
	DEVMETHOD(device_detach,	coretemp_detach),

	{0, 0}
};

static driver_t coretemp_driver = {
	"coretemp",
	coretemp_methods,
	sizeof(struct coretemp_softc),
};

static devclass_t coretemp_devclass;
DRIVER_MODULE(coretemp, cpu, coretemp_driver, coretemp_devclass, NULL, NULL);

static void
coretemp_identify(driver_t *driver, device_t parent)
{
	device_t child;
	u_int regs[4];

	/* Make sure we're not being doubly invoked. */
	if (device_find_child(parent, "coretemp", -1) != NULL)
		return;

	/* Check that CPUID is supported and the vendor is Intel.*/
	if (cpu_high == 0 || strcmp(cpu_vendor, "GenuineIntel"))
		return;
	/*
	 * CPUID 0x06 returns 1 if the processor has on-die thermal
	 * sensors. EBX[0:3] contains the number of sensors.
	 */
	do_cpuid(0x06, regs);
	if ((regs[0] & 0x1) != 1)
		return;

	/*
	 * We add a child for each CPU since settings must be performed
	 * on each CPU in the SMP case.
	 */
	child = device_add_child(parent, "coretemp", -1);
	if (child == NULL)
		device_printf(parent, "add coretemp child failed\n");
}

static int
coretemp_probe(device_t dev)
{
	if (resource_disabled("coretemp", 0))
		return (ENXIO);

	device_set_desc(dev, "CPU On-Die Thermal Sensors");

	return (BUS_PROBE_GENERIC);
}

static int
coretemp_attach(device_t dev)
{
	struct coretemp_softc *sc = device_get_softc(dev);
	device_t pdev;
	uint64_t msr;
	int cpu_model;
	int cpu_mask;

	sc->sc_dev = dev;
	pdev = device_get_parent(dev);
	cpu_model = (cpu_id >> 4) & 15;
	/* extended model */
	cpu_model += ((cpu_id >> 16) & 0xf) << 4;
	cpu_mask = cpu_id & 15;

	/*
	 * Check for errata AE18.
	 * "Processor Digital Thermal Sensor (DTS) Readout stops
	 *  updating upon returning from C3/C4 state."
	 *
	 * Adapted from the Linux coretemp driver.
	 */
	if (cpu_model == 0xe && cpu_mask < 0xc) {
		msr = rdmsr(MSR_BIOS_SIGN);
		msr = msr >> 32;
		if (msr < 0x39) {
			device_printf(dev, "not supported (Intel errata "
			    "AE18), try updating your BIOS\n");
			return (ENXIO);
		}
	}
	/*
	 * On some Core 2 CPUs, there's an undocumented MSR that
	 * can tell us if Tj(max) is 100 or 85.
	 *
	 * The if-clause for CPUs having the MSR_IA32_EXT_CONFIG was adapted
	 * from the Linux coretemp driver.
	 */
	sc->sc_tjmax = 100;
	if ((cpu_model == 0xf && cpu_mask >= 2) || cpu_model == 0xe) {
		msr = rdmsr(MSR_IA32_EXT_CONFIG);
		if (msr & (1 << 30))
			sc->sc_tjmax = 85;
	}

	/*
	 * Add hw.sensors.cpuN.temp0 MIB.
	 */
	strlcpy(sc->sc_sensordev.xname, device_get_nameunit(pdev),
	    sizeof(sc->sc_sensordev.xname));
	sc->sc_sensor.type = SENSOR_TEMP;
	sensor_attach(&sc->sc_sensordev, &sc->sc_sensor);
	if (sensor_task_register(sc, coretemp_refresh, 2)) {
		device_printf(dev, "unable to register update task\n");
		return (ENXIO);
	}
	sensordev_install(&sc->sc_sensordev);

	return (0);
}

static int
coretemp_detach(device_t dev)
{
	struct coretemp_softc *sc = device_get_softc(dev);

	sensordev_deinstall(&sc->sc_sensordev);
	sensor_task_unregister(sc);

	return (0);
}


static int
coretemp_get_temp(device_t dev)
{
	uint64_t msr;
	int temp;
	int cpu = device_get_unit(dev);
	struct coretemp_softc *sc = device_get_softc(dev);
	char stemp[16];

	/*
	 * Bind to specific CPU to read the correct temperature.
	 * If not all CPUs are initialised, then only read from
	 * cpu0, returning -1 on all other CPUs.
	 */
	if (smp_cpus > 1) {
		thread_lock(curthread);
		sched_bind(curthread, cpu);
		msr = rdmsr(MSR_THERM_STATUS);
		sched_unbind(curthread);
		thread_unlock(curthread);
	} else if (cpu != 0)
		return (-1);
	else
		msr = rdmsr(MSR_THERM_STATUS);

	/*
	 * Check for Thermal Status and Thermal Status Log.
	 */
	if ((msr & 0x3) == 0x3)
		device_printf(dev, "PROCHOT asserted\n");

	/*
	 * Bit 31 contains "Reading valid"
	 */
	if (((msr >> 31) & 0x1) == 1) {
		/*
		 * Starting on bit 16 and ending on bit 22.
		 */
		temp = sc->sc_tjmax - ((msr >> 16) & 0x7f);
	} else
		temp = -1;

	/*
	 * Check for Critical Temperature Status and Critical
	 * Temperature Log.
	 * It doesn't really matter if the current temperature is
	 * invalid because the "Critical Temperature Log" bit will
	 * tell us if the Critical Temperature has been reached in
	 * past. It's not directly related to the current temperature.
	 *
	 * If we reach a critical level, allow devctl(4) to catch this
	 * and shutdown the system.
	 */
	if (((msr >> 4) & 0x3) == 0x3) {
		device_printf(dev, "critical temperature detected, "
		    "suggest system shutdown\n");
		snprintf(stemp, sizeof(stemp), "%d", temp);
		devctl_notify("coretemp", "Thermal", stemp, "notify=0xcc");
	}

	return (temp);
}

static void
coretemp_refresh(void *arg)
{
	struct coretemp_softc *sc = arg;
	device_t dev = sc->sc_dev;
	struct ksensor *s = &sc->sc_sensor;
	int temp;

	temp = coretemp_get_temp(dev);

	if (temp == -1) {
		s->flags |= SENSOR_FINVALID;
		s->value = 0;
	} else {
		s->flags &= ~SENSOR_FINVALID;
		s->value = temp * 1000000 + 273150000;
	}
}
