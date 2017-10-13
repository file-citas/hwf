; ModuleID = 'smem-write-byval.c'
source_filename = "smem-write-byval.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@smem = common global [1024 x i32] zeroinitializer, align 16, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define void @write_shm(i32 %data, i32 %offset) #0 !dbg !16 {
entry:
  %data.addr = alloca i32, align 4
  %offset.addr = alloca i32, align 4
  store i32 %data, i32* %data.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %data.addr, metadata !19, metadata !20), !dbg !21
  store i32 %offset, i32* %offset.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %offset.addr, metadata !22, metadata !20), !dbg !23
  %0 = load i32, i32* %data.addr, align 4, !dbg !24
  %1 = load i32, i32* %offset.addr, align 4, !dbg !25
  %idx.ext = zext i32 %1 to i64, !dbg !26
  %add.ptr = getelementptr inbounds i32, i32* getelementptr inbounds ([1024 x i32], [1024 x i32]* @smem, i32 0, i32 0), i64 %idx.ext, !dbg !26
  store i32 %0, i32* %add.ptr, align 4, !dbg !27
  ret void, !dbg !28
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 !dbg !29 {
entry:
  %retval = alloca i32, align 4
  %data = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 1, i32* getelementptr inbounds ([1024 x i32], [1024 x i32]* @smem, i64 0, i64 0), align 16, !dbg !33
  call void @llvm.dbg.declare(metadata i32* %data, metadata !34, metadata !20), !dbg !35
  store i32 5, i32* %data, align 4, !dbg !35
  %0 = load i32, i32* %data, align 4, !dbg !36
  call void @write_shm(i32 %0, i32 0), !dbg !37
  ret i32 0, !dbg !38
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!12, !13, !14}
!llvm.ident = !{!15}

!0 = !DIGlobalVariableExpression(var: !1)
!1 = distinct !DIGlobalVariable(name: "smem", scope: !2, file: !3, line: 4, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 5.0.0 (tags/RELEASE_500/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "smem-write-byval.c", directory: "/home/dokyungs/hwf/driver/llvm/test/Analysis")
!4 = !{}
!5 = !{!0}
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 32768, elements: !10)
!7 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint32_t", file: !8, line: 51, baseType: !9)
!8 = !DIFile(filename: "/usr/include/stdint.h", directory: "/home/dokyungs/hwf/driver/llvm/test/Analysis")
!9 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!10 = !{!11}
!11 = !DISubrange(count: 1024)
!12 = !{i32 2, !"Dwarf Version", i32 4}
!13 = !{i32 2, !"Debug Info Version", i32 3}
!14 = !{i32 1, !"wchar_size", i32 4}
!15 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
!16 = distinct !DISubprogram(name: "write_shm", scope: !3, file: !3, line: 7, type: !17, isLocal: false, isDefinition: true, scopeLine: 7, flags: DIFlagPrototyped, isOptimized: false, unit: !2, variables: !4)
!17 = !DISubroutineType(types: !18)
!18 = !{null, !7, !7}
!19 = !DILocalVariable(name: "data", arg: 1, scope: !16, file: !3, line: 7, type: !7)
!20 = !DIExpression()
!21 = !DILocation(line: 7, column: 25, scope: !16)
!22 = !DILocalVariable(name: "offset", arg: 2, scope: !16, file: !3, line: 7, type: !7)
!23 = !DILocation(line: 7, column: 40, scope: !16)
!24 = !DILocation(line: 8, column: 22, scope: !16)
!25 = !DILocation(line: 8, column: 12, scope: !16)
!26 = !DILocation(line: 8, column: 10, scope: !16)
!27 = !DILocation(line: 8, column: 20, scope: !16)
!28 = !DILocation(line: 9, column: 1, scope: !16)
!29 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 12, type: !30, isLocal: false, isDefinition: true, scopeLine: 12, isOptimized: false, unit: !2, variables: !4)
!30 = !DISubroutineType(types: !31)
!31 = !{!32}
!32 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!33 = !DILocation(line: 13, column: 11, scope: !29)
!34 = !DILocalVariable(name: "data", scope: !29, file: !3, line: 14, type: !7)
!35 = !DILocation(line: 14, column: 12, scope: !29)
!36 = !DILocation(line: 15, column: 13, scope: !29)
!37 = !DILocation(line: 15, column: 3, scope: !29)
!38 = !DILocation(line: 16, column: 3, scope: !29)
