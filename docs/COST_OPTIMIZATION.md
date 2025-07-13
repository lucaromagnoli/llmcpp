# GitHub Actions Cost Optimization

## Overview

This document outlines the cost optimization strategies implemented to reduce GitHub Actions spending, particularly focusing on reducing expensive macOS build costs.

## Cost Analysis

Based on the provided cost breakdown:
- **Linux**: $4.45 (556 min)
- **macOS**: $55.86 (698.3 min) - **12.5x more expensive**
- **Windows**: $14.14 (884 min)

macOS builds are significantly more expensive due to higher per-minute rates and longer build times.

## Implemented Optimizations

### 1. Removed macOS from Regular CI

**Before**: macOS builds ran on every push and PR
**After**: macOS builds only run on releases

- **File**: `.github/workflows/ci.yml`
- **Impact**: Eliminates ~$55 per CI run
- **Risk**: Lower macOS coverage for regular development

### 2. Created Separate macOS CI Workflow

**File**: `.github/workflows/macos-ci.yml`

- **Manual trigger**: `workflow_dispatch`
- **Automatic trigger**: Only on main branch pushes (excluding docs)
- **Purpose**: Allows macOS testing when needed without high costs

### 3. Improved Caching Strategy

**Enhanced cache keys** to include source files:
```yaml
key: cmake-${{ matrix.os }}-${{ matrix.build_type }}-${{ hashFiles('CMakeLists.txt', '**/CMakeLists.txt', 'src/**/*', 'include/**/*') }}
```

**Separate cache keys** for releases vs CI to avoid conflicts.

### 4. Added Timeouts

**Build timeouts**: 15-20 minutes (was unlimited)
**Test timeouts**: 10 minutes
**Job timeouts**: 30 minutes

Prevents runaway builds that consume excessive minutes.

### 5. Path-Based Triggers

**Skip CI for documentation changes**:
```yaml
paths-ignore:
  - '**.md'
  - 'docs/**'
  - '.github/**'
  - 'LICENSE'
```

### 6. Optimized Release Workflow

- **Better caching** for release builds
- **Reduced timeouts** for faster failure detection
- **Separate cache keys** to avoid conflicts with CI

## Expected Cost Savings

### Per CI Run
- **Before**: ~$74.45 (Linux + macOS + Windows)
- **After**: ~$18.59 (Linux + Windows only)
- **Savings**: ~75% reduction

### Per Month (assuming 20 CI runs)
- **Before**: ~$1,489
- **After**: ~$372
- **Savings**: ~$1,117 per month

## Trade-offs

### Pros
- Significant cost reduction
- Faster CI feedback (fewer platforms)
- Better resource utilization

### Cons
- Reduced macOS coverage for regular development
- Potential for macOS-specific issues to slip through
- Manual intervention needed for macOS testing

## Recommendations

### For Development
1. **Use Linux as primary development platform**
2. **Run macOS CI manually** when making macOS-specific changes
3. **Test on macOS locally** before pushing

### For Releases
1. **Keep all platforms** for release builds (quality assurance)
2. **Monitor release build times** and optimize further if needed

### Future Optimizations
1. **Consider using self-hosted runners** for macOS builds
2. **Implement conditional macOS builds** based on file changes
3. **Use GitHub's larger runners** for faster builds (if cost-effective)

## Monitoring

Track the following metrics:
- **Monthly Actions costs**
- **Build times per platform**
- **Cache hit rates**
- **Failed builds due to platform-specific issues**

## Rollback Plan

If issues arise:
1. **Re-enable macOS in regular CI** by reverting `.github/workflows/ci.yml`
2. **Remove the separate macOS workflow** if not needed
3. **Monitor for platform-specific regressions**

## Commands

### Manual macOS CI Trigger
```bash
# Via GitHub CLI
gh workflow run macos-ci.yml

# Or via GitHub web interface
# Actions → macOS CI → Run workflow
```

### Check Current Costs
```bash
# View Actions usage in GitHub
# Settings → Billing → Actions
```
