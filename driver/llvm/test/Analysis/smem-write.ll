; ModuleID = 'smem-write.c'
source_filename = "smem-write.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.hfi_cmd_fake = type { i32 }

@smem = common global [1024 x i32] zeroinitializer, align 16, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define void @send_packet(i8* %pkt) #0 !dbg !20 {
entry:
  %pkt.addr = alloca i8*, align 8
  store i8* %pkt, i8** %pkt.addr, align 8
  call void @llvm.dbg.declare(metadata i8** %pkt.addr, metadata !23, metadata !24), !dbg !25
  %0 = load i8*, i8** %pkt.addr, align 8, !dbg !26
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast ([1024 x i32]* @smem to i8*), i8* %0, i64 4, i32 1, i1 false), !dbg !27
  ret void, !dbg !28
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #2

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 !dbg !29 {
entry:
  %retval = alloca i32, align 4
  %pkt = alloca %struct.hfi_cmd_fake, align 4
  store i32 0, i32* %retval, align 4
  store i32 1, i32* getelementptr inbounds ([1024 x i32], [1024 x i32]* @smem, i64 0, i64 0), align 16, !dbg !33
  call void @llvm.dbg.declare(metadata %struct.hfi_cmd_fake* %pkt, metadata !34, metadata !24), !dbg !38
  %size = getelementptr inbounds %struct.hfi_cmd_fake, %struct.hfi_cmd_fake* %pkt, i32 0, i32 0, !dbg !39
  store i32 5, i32* %size, align 4, !dbg !40
  %0 = bitcast %struct.hfi_cmd_fake* %pkt to i8*, !dbg !41
  call void @send_packet(i8* %0), !dbg !42
  ret i32 0, !dbg !43
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!16, !17, !18}
!llvm.ident = !{!19}

!0 = !DIGlobalVariableExpression(var: !1)
!1 = distinct !DIGlobalVariable(name: "smem", scope: !2, file: !3, line: 8, type: !11, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 5.0.0 (tags/RELEASE_500/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !5, globals: !10)
!3 = !DIFile(filename: "smem-write.c", directory: "/home/dokyungs/hwf/driver/llvm/test/Analysis")
!4 = !{}
!5 = !{!6}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!7 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint8_t", file: !8, line: 48, baseType: !9)
!8 = !DIFile(filename: "/usr/include/stdint.h", directory: "/home/dokyungs/hwf/driver/llvm/test/Analysis")
!9 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!10 = !{!0}
!11 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 32768, elements: !14)
!12 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint32_t", file: !8, line: 51, baseType: !13)
!13 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!14 = !{!15}
!15 = !DISubrange(count: 1024)
!16 = !{i32 2, !"Dwarf Version", i32 4}
!17 = !{i32 2, !"Debug Info Version", i32 3}
!18 = !{i32 1, !"wchar_size", i32 4}
!19 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
!20 = distinct !DISubprogram(name: "send_packet", scope: !3, file: !3, line: 11, type: !21, isLocal: false, isDefinition: true, scopeLine: 11, flags: DIFlagPrototyped, isOptimized: false, unit: !2, variables: !4)
!21 = !DISubroutineType(types: !22)
!22 = !{null, !6}
!23 = !DILocalVariable(name: "pkt", arg: 1, scope: !20, file: !3, line: 11, type: !6)
!24 = !DIExpression()
!25 = !DILocation(line: 11, column: 27, scope: !20)
!26 = !DILocation(line: 12, column: 28, scope: !20)
!27 = !DILocation(line: 12, column: 3, scope: !20)
!28 = !DILocation(line: 13, column: 1, scope: !20)
!29 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 16, type: !30, isLocal: false, isDefinition: true, scopeLine: 16, isOptimized: false, unit: !2, variables: !4)
!30 = !DISubroutineType(types: !31)
!31 = !{!32}
!32 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!33 = !DILocation(line: 17, column: 11, scope: !29)
!34 = !DILocalVariable(name: "pkt", scope: !29, file: !3, line: 18, type: !35)
!35 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "hfi_cmd_fake", file: !3, line: 4, size: 32, elements: !36)
!36 = !{!37}
!37 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !35, file: !3, line: 5, baseType: !12, size: 32)
!38 = !DILocation(line: 18, column: 23, scope: !29)
!39 = !DILocation(line: 19, column: 7, scope: !29)
!40 = !DILocation(line: 19, column: 12, scope: !29)
!41 = !DILocation(line: 20, column: 15, scope: !29)
!42 = !DILocation(line: 20, column: 3, scope: !29)
!43 = !DILocation(line: 21, column: 3, scope: !29)
