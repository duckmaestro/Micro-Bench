/*
 * This file is part of Micro Bench
 * <https://github.com/duckmaestro/Micro-Bench>.
 * 
 * Micro Bench is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Micro Bench is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#pragma once

// uber include for all experiments

// CPU, Schedulding, OS Services
#include "MeasurementOverhead.h"
#include "ProcedureOverhead.h"
#include "SystemCallOverhead.h"
#include "TaskCreation.h"
#include "ContextSwitchTime.h"

// Memory
#include "MemoryLatency.h"
#include "MemoryBandwidth.h"
#include "MemoryPageFault.h"

// Network
#include "NetworkRoundTripTime.h"
#include "NetworkPeakBandwidth.h"
#include "NetworkSetupTeardown.h"

// Disk
#include "FileSystemCacheSize.h"
#include "FileSystemLocalRead.h"
#include "FileSystemRemoteRead.h"
#include "FileSystemContention.h"

// Misc
#include "FakeKernel.h"
#include "LoopOverhead.h"
