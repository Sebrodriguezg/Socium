// Genera el Documento Maestro de Especificación (DOCX) del simulador ABM Colombia.
// Requiere la librería `docx`. Ejecutar con node, p.ej.:
//   NODE_PATH=/tmp/docxgen/node_modules node scripts/generar_spec_docx.js
// o instalar docx en el repo (corepack npm install docx) y: node scripts/generar_spec_docx.js
// Salida: docs/spec/ABM_Colombia_Especificacion_Maestra.docx
const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell,
  HeadingLevel, AlignmentType, BorderStyle, WidthType, ShadingType,
  LevelFormat, PageNumber, PageBreak, Header, Footer, TabStopType,
  TabStopPosition, PositionalTab, PositionalTabAlignment, PositionalTabRelativeTo,
  PositionalTabLeader
} = require('docx');
const fs = require('fs');

// ─── COLORS ──────────────────────────────────────────────────────────────────
const C = {
  dark:    "1A1A2E",
  accent:  "16213E",
  gold:    "E94560",
  mid:     "0F3460",
  light:   "E8F4FD",
  border:  "BBCFE8",
  tblHead: "1A3A5C",
  tblAlt:  "EEF4FB",
};

// ─── HELPERS ─────────────────────────────────────────────────────────────────
const br = (n=1) => Array(n).fill(0).map(() =>
  new Paragraph({ children: [new TextRun("")], spacing:{after:0,before:0} })
);

function h1(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_1,
    children: [new TextRun({ text, font:"Arial", size:32, bold:true, color:C.dark })],
    spacing:{before:360, after:160},
    border:{ bottom:{ style:BorderStyle.THICK, size:6, color:C.gold, space:4 } }
  });
}

function h2(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_2,
    children: [new TextRun({ text, font:"Arial", size:26, bold:true, color:C.mid })],
    spacing:{before:280, after:120}
  });
}

function h3(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_3,
    children: [new TextRun({ text, font:"Arial", size:22, bold:true, color:C.accent })],
    spacing:{before:200, after:80}
  });
}

function body(text, opts={}) {
  return new Paragraph({
    children: [new TextRun({ text, font:"Arial", size:20, color:"222222", ...opts })],
    spacing:{before:60, after:80},
    alignment: AlignmentType.JUSTIFIED
  });
}

function bodyItalic(text) {
  return body(text, { italics:true, color:"444466" });
}

function bullet(text, level=0) {
  return new Paragraph({
    numbering:{ reference:"bullets", level },
    children: [new TextRun({ text, font:"Arial", size:20, color:"222222" })],
    spacing:{before:40, after:60}
  });
}

function formula(text) {
  return new Paragraph({
    children: [new TextRun({ text, font:"Courier New", size:20, color:C.accent, bold:true })],
    spacing:{before:100, after:100},
    alignment: AlignmentType.CENTER,
    shading:{ fill:C.light, type:ShadingType.CLEAR }
  });
}

function infoBox(label, text) {
  const border = { style:BorderStyle.SINGLE, size:1, color:C.border };
  return new Table({
    width:{size:9360, type:WidthType.DXA},
    columnWidths:[1440,7920],
    rows:[new TableRow({ children:[
      new TableCell({
        width:{size:1440, type:WidthType.DXA},
        borders:{top:border,bottom:border,left:border,right:border},
        shading:{fill:C.tblHead, type:ShadingType.CLEAR},
        margins:{top:100,bottom:100,left:120,right:120},
        children:[new Paragraph({children:[new TextRun({text:label,font:"Arial",size:18,bold:true,color:"FFFFFF"})],alignment:AlignmentType.CENTER})]
      }),
      new TableCell({
        width:{size:7920, type:WidthType.DXA},
        borders:{top:border,bottom:border,left:border,right:border},
        shading:{fill:C.light, type:ShadingType.CLEAR},
        margins:{top:100,bottom:100,left:180,right:120},
        children:[new Paragraph({children:[new TextRun({text,font:"Arial",size:19,color:"1A1A2E"})],alignment:AlignmentType.JUSTIFIED})]
      })
    ]})]
  });
}

function dataTable(headers, rows) {
  const border = { style:BorderStyle.SINGLE, size:1, color:C.border };
  const colW = Math.floor(9360 / headers.length);
  const colWidths = headers.map(() => colW);

  function makeRow(cells, isHeader, isAlt) {
    return new TableRow({ children: cells.map((c, i) =>
      new TableCell({
        width:{size:colWidths[i], type:WidthType.DXA},
        borders:{top:border,bottom:border,left:border,right:border},
        shading:{fill: isHeader ? C.tblHead : isAlt ? C.tblAlt : "FFFFFF", type:ShadingType.CLEAR},
        margins:{top:80,bottom:80,left:120,right:80},
        children:[new Paragraph({children:[new TextRun({text:c,font:"Arial",size:isHeader?18:18,bold:isHeader,color:isHeader?"FFFFFF":"222222"})],alignment:AlignmentType.LEFT})]
      })
    )});
  }

  return new Table({
    width:{size:9360, type:WidthType.DXA},
    columnWidths: colWidths,
    rows:[
      makeRow(headers, true, false),
      ...rows.map((r,i) => makeRow(r, false, i%2===0))
    ]
  });
}

// ─── DOCUMENT CONTENT ────────────────────────────────────────────────────────

const doc = new Document({
  styles:{
    default:{ document:{ run:{ font:"Arial", size:20 } } },
    paragraphStyles:[
      { id:"Heading1", name:"Heading 1", basedOn:"Normal", next:"Normal", quickFormat:true,
        run:{ size:32, bold:true, font:"Arial", color:C.dark },
        paragraph:{ spacing:{before:360,after:160}, outlineLevel:0 } },
      { id:"Heading2", name:"Heading 2", basedOn:"Normal", next:"Normal", quickFormat:true,
        run:{ size:26, bold:true, font:"Arial", color:C.mid },
        paragraph:{ spacing:{before:280,after:120}, outlineLevel:1 } },
      { id:"Heading3", name:"Heading 3", basedOn:"Normal", next:"Normal", quickFormat:true,
        run:{ size:22, bold:true, font:"Arial", color:C.accent },
        paragraph:{ spacing:{before:200,after:80}, outlineLevel:2 } },
    ]
  },
  numbering:{
    config:[{
      reference:"bullets",
      levels:[
        { level:0, format:LevelFormat.BULLET, text:"•",
          alignment:AlignmentType.LEFT,
          style:{ paragraph:{ indent:{ left:720, hanging:360 } } } },
        { level:1, format:LevelFormat.BULLET, text:"◦",
          alignment:AlignmentType.LEFT,
          style:{ paragraph:{ indent:{ left:1080, hanging:360 } } } },
      ]
    }]
  },
  sections:[{
    properties:{
      page:{ size:{ width:12240, height:15840 }, margin:{ top:1080, right:1080, bottom:1080, left:1080 } }
    },
    headers:{ default: new Header({ children:[
      new Paragraph({
        children:[new TextRun({text:"DOCUMENTO MAESTRO — SIMULADOR ABM COLOMBIA  |  CONFIDENCIAL — SEBASTIÁN", font:"Arial", size:16, color:"888888"})],
        alignment:AlignmentType.RIGHT,
        border:{bottom:{style:BorderStyle.SINGLE,size:2,color:C.border,space:4}}
      })
    ]}) },
    footers:{ default: new Footer({ children:[
      new Paragraph({
        children:[
          new TextRun({text:"Página ", font:"Arial", size:16, color:"888888"}),
          new TextRun({ children:[PageNumber.CURRENT], font:"Arial", size:16, color:"888888" }),
          new TextRun({text:" de ", font:"Arial", size:16, color:"888888"}),
          new TextRun({ children:[PageNumber.TOTAL_PAGES], font:"Arial", size:16, color:"888888" }),
          new TextRun({text:"  |  Versión 1.0 — Junio 2026", font:"Arial", size:16, color:"888888"})
        ],
        border:{top:{style:BorderStyle.SINGLE,size:2,color:C.border,space:4}}
      })
    ]}) },
    children: [

      // ═══════════════════════════════════════════════════════
      // PORTADA
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new TextRun({text:"", font:"Arial", size:20})], spacing:{before:600,after:0} }),
      new Paragraph({
        children:[new TextRun({text:"SIMULADOR NACIONAL ABM", font:"Arial", size:56, bold:true, color:C.dark})],
        alignment:AlignmentType.CENTER, spacing:{before:0,after:120}
      }),
      new Paragraph({
        children:[new TextRun({text:"COLOMBIA — GEMELO DIGITAL ESTOCÁSTICO", font:"Arial", size:36, bold:false, color:C.mid})],
        alignment:AlignmentType.CENTER, spacing:{before:0,after:60}
      }),
      new Paragraph({
        border:{ bottom:{style:BorderStyle.THICK, size:8, color:C.gold, space:8} },
        children:[new TextRun("")], spacing:{before:0,after:240}
      }),
      new Paragraph({
        children:[new TextRun({text:"Documento Maestro de Especificación Técnica", font:"Arial", size:26, italics:true, color:"555555"})],
        alignment:AlignmentType.CENTER, spacing:{before:0,after:80}
      }),
      new Paragraph({
        children:[new TextRun({text:"Parámetros · Probabilidades Calibradas · Arquitectura Computacional · Condiciones de Iteración", font:"Arial", size:20, italics:true, color:"777777"})],
        alignment:AlignmentType.CENTER, spacing:{before:0,after:600}
      }),
      new Paragraph({
        children:[new TextRun({text:"Elaborado para: Sebastián  |  Junio 2026", font:"Arial", size:20, color:"555555"})],
        alignment:AlignmentType.CENTER, spacing:{before:0,after:80}
      }),
      new Paragraph({
        children:[new TextRun({text:"Fuentes: DANE · Banco de la República · UNODC · UNHCR · WHO · World Bank · arxiv.org", font:"Arial", size:18, color:"888888"})],
        alignment:AlignmentType.CENTER, spacing:{before:0,after:0}
      }),
      new Paragraph({ children:[new TextRun({text:"", size:20})], pageBreakBefore:true }),

      // ═══════════════════════════════════════════════════════
      // 0. ADVERTENCIA PRELIMINAR
      // ═══════════════════════════════════════════════════════
      h1("0. ADVERTENCIA DE ALCANCE Y RIGOR"),
      body("Este documento condensa en un solo marco de referencia todas las condiciones que la literatura científica de frontera exige para construir un ABM estocástico y espacialmente explícito a escala nacional. Cada probabilidad de transición citada proviene de fuentes primarias calibradas con datos reales de Colombia (DANE, Banco de la República, UNODC, UNHCR, Ministerio de Salud) o de la literatura de modelado con aprendizaje automático más reciente disponible a junio de 2026. El documento debe leerse como una especificación ejecutable, no como una revisión literaria."),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 1. FUNDAMENTOS MATEMÁTICOS
      // ═══════════════════════════════════════════════════════
      h1("1. FUNDAMENTOS MATEMÁTICOS DEL SUSTRATO"),

      h2("1.1 Ecuación Maestra del Sistema"),
      body("El estado global del sistema es el vector X que describe la configuración simultánea de todos los N ~ 50 millones de agentes. Su evolución probabilística obedece a la Ecuación Maestra:"),
      formula("dP(X,t)/dt = SUM_{X'} [ W(X|X')·P(X',t) - W(X'|X)·P(X,t) ]"),
      body("donde W(X|X') es la tasa de transición por unidad de tiempo del microestado X' al X. Todo el edificio del simulador consiste en calcular estas tasas W de forma computacionalmente tratable."),

      h2("1.2 Geometría Diferencial del Territorio"),
      body("Colombia no puede tratarse como una grilla cartesiana 2D plana. La orografía andina, los ríos navegables y la infraestructura vial deforman el espacio de movilidad. El territorio es una variedad riemanniana con tensor métrico dinámico g_uv(r,t):"),
      formula("ds² = g_μν(r,t) dx^μ dx^ν"),
      body("Las componentes diagonales g_11, g_22 aumentan drásticamente en cordilleras (p.ej. cruzar el Alto de La Línea equivale a ~8h de penalización temporal) y disminuyen donde existen vías 4G. Las trayectorias óptimas de migración de agentes son geodésicas que minimizan:"),
      formula("S = INTEGRAL sqrt(g_μν · (dx^μ/dτ)(dx^ν/dτ)) dτ"),
      body("Implementación: extraer Modelos de Elevación Digital (DEM) del IGAC/NASA SRTM (resolución 30m), mapear gradiente topográfico nabla·z(x,y) a componentes del tensor mediante transformación conforme. La resistencia vial se añade como campo escalar aditivo sobre g_11, g_22."),

      h2("1.3 Dinámica de Langevin (Variables Continuas)"),
      body("Para variables de estado continuas del agente i (capital financiero w_i, nivel de radicalización theta_i, estado de salud s_i), la actualización temporal sigue una Ecuación Diferencial Estocástica de Itô:"),
      formula("dw_i(t) = μ(w_i, r, t)dt + σ(w_i, r, t)dW_i(t)"),
      body("donde dW_i(t) es proceso de Wiener estándar: media cero, varianza cruzada <dW_i·dW_j> = δ_ij·δ(t-t')·dt. NOTA CRÍTICA: usar prescripción de Stratonovich cuando las 'fuerzas' económicas tienen tiempo de correlación finito antes de converger al límite de ruido blanco, o aplicar la corrección de drift correspondiente para no violar la conservación de capital a nivel macro."),

      h2("1.4 Ecuación de Fokker-Planck (Distribución de Riqueza)"),
      body("La concentración espacial del capital a nivel poblacional debe reproducir empíricamente una distribución de Gibbs-Boltzmann para rentas bajas y una ley de potencia de Pareto para rentas altas. Esto emerge naturalmente de la ecuación:"),
      formula("∂P(w,t)/∂t = -∂/∂w[D_1(w)·P(w,t)] + (1/2)∂²/∂w²[D_2(w)·P(w,t)]"),
      body("Calibración empírica Colombia: Gini Index 2023 = 53.9 (World Bank). El coeficiente de Pareto para el 1% más rico de Colombia se estima en alpha ~ 1.8-2.0, consistente con datos históricos del Banco de la República. El 10% más rico concentra ~40% del ingreso nacional."),

      h2("1.5 Decisiones por Función de Partición (Racionalidad Limitada)"),
      body("Todas las decisiones discretas del agente (delinquir, migrar, formalizarse, votar) siguen el formalismo de maximización de utilidad esperada con ruido estocástico — análogo a la distribución de Boltzmann en mecánica estadística:"),
      formula("P(acción a) = exp(-β·U(a)) / SUM_{a' ∈ A} exp(-β·U(a'))"),
      body("donde U(a) es la función de utilidad/costo de la acción y β parametriza la racionalidad del agente (β→∞: racional perfecto; β→0: aleatorio puro). Para Colombia, β debe calibrarse por grupo socioeconómico — los agentes en estrato 1-2 bajo estrés de violencia exhiben β efectivo menor (mayor ruido decisional)."),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 2. VECTOR DE ESTADO DEL AGENTE INDIVIDUAL
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new PageBreak()], spacing:{before:0,after:0} }),
      h1("2. VECTOR DE ESTADO DEL AGENTE: INDIVIDUO"),
      body("Cada agente i porta el vector S_i(t) con las siguientes dimensiones. Se recomiendan al menos 60 variables activas por agente para capturar la heterogeneidad relevante sin exceder los 80-100 GB de RAM para N=50M agentes en precisión simple (float32)."),
      ...br(1),

      h2("2.1 Localización y Demografía"),
      dataTable(
        ["Variable","Tipo","Rango / Valores","Fuente de Calibración"],
        [
          ["Coordenadas (lat,lon,alt)","float32","Colombia: lat [-4.2, 12.5], lon [-79.0, -66.8], alt [0,5775m]","IGAC, SRTM DEM"],
          ["Municipio de residencia","int16","1099 municipios DANE","DIVIPOLA DANE"],
          ["Municipio de origen","int16","Ídem","Registro Único de Víctimas (RUV)"],
          ["Estatus desplazamiento","bool","0=no, 1=desplazado interno","UARIV — 5.5M IDPs (2019)"],
          ["Edad biológica","int8","0-105 años","Tablas de vida DANE 2020-2025"],
          ["Sexo biológico","bool","0=M, 1=F","DANE CNPV 2018: 51.2% F"],
          ["Etnia auto-reconocida","int8","0=mestizo, 1=afrocolombiano, 2=indígena, 3=raizal, 4=palenquero, 5=rom","DANE: 9.34% afro, 4.4% indígena"],
          ["Tamaño del hogar","int8","1-15 personas","GEIH 2023: media 3.1 personas/hogar"],
        ]
      ),

      h2("2.2 Estratificación y Capital Social"),
      dataTable(
        ["Variable","Tipo","Distribución Calibrada Colombia","Fuente"],
        [
          ["Estrato vivienda","int8","1: 23%, 2: 24%, 3: 27%, 4: 12%, 5: 6%, 6: 4%, sin estrato: 4%","DANE SBS 2022"],
          ["Puntaje SISBEN IV","float32","A: <30 pts, B: 30-57.5, C: 57.5-86, D: >86","DNP 2022 — 68% en A y B"],
          ["Tipo tenencia vivienda","int8","0=propia pagada (37%), 1=propia pagando (11%), 2=arriendo (39%), 3=usufructo/invasión (13%)","DANE CNPV 2018"],
          ["Hacinamiento crítico","bool","P(hacinamiento|estrato 1)=0.28, P(hacinamiento|estrato 6)=0.01","DANE ECV 2022"],
          ["Red de soporte social (grado k)","int16","Distribución power-law P(k)~k^(-gamma), gamma~2.5 (Barabási-Albert)","Literatura redes sociales"],
        ]
      ),

      h2("2.3 Capital Humano y Educación"),
      dataTable(
        ["Variable","Tipo","Distribución Calibrada Colombia","Fuente"],
        [
          ["Años de escolaridad","float32","Media nacional: 9.2 años (urbano: 10.8, rural: 6.4)","DANE GEIH 2023"],
          ["Nivel máximo alcanzado","int8","0=ninguno(7%), 1=primaria(28%), 2=secundaria(33%), 3=media(16%), 4=superior(16%)","MEN 2022"],
          ["Analfabetismo","bool","P(analfabeto) = 0.045 nacional; rural: 0.107; urbano: 0.025","DANE CNPV 2018"],
          ["Calidad inst. educativa","float32","Percentil en pruebas Saber 11 (0-100), correlacionado con estrato (r~0.62)","ICFES 2022"],
          ["Deserción escolar acumulada","bool","P(deserción antes de grado 9 | estrato 1) = 0.24","MEN tasa deserción 2022"],
        ]
      ),

      h2("2.4 Estatus Laboral y Capital Financiero"),
      dataTable(
        ["Variable","Tipo","Distribución Calibrada Colombia","Fuente"],
        [
          ["Condición de actividad","int8","Ocupado: 56.1%, Desocupado: 9.9%, Inactivo: 34%","DANE GEIH enero 2024"],
          ["Formalidad laboral","bool","Informal: 56.0% nacional; urbano: 43%; rural: 84.7%","Banrep/DANE 2023"],
          ["Sector CIIU principal","int8","Agro(16%), Comercio(27%), Industria(12%), Servicios(25%), Construc(6%), Otros(14%)","DANE GEIH 2023"],
          ["Ingreso mensual neto (COP)","float32","Mediana: $1.160.000; Pareto alpha~1.9 para top 5%","DANE GEIH 2023"],
          ["Endeudamiento informal (gota a gota)","float32","Prevalencia en estrato 1-2: ~35%; tasa efectiva promedio: 180%-360% EA","Superfinanciera 2022"],
          ["Historial crediticio formal","float32","Score Datacrédito normalizado 0-1; media 0.52 para personas con historial","Transunion Colombia 2022"],
        ]
      ),

      h2("2.5 Salud y Morbilidad"),
      dataTable(
        ["Variable","Tipo","Distribución / Parámetros Colombia","Fuente"],
        [
          ["Estado nutricional","int8","0=normal, 1=desnutrición crónica (18.8% niños <5 años), 2=obesidad (22.3% adultos)","ENSIN 2015, DANE"],
          ["Comorbilidades crónicas","bitmask","Hipertensión: 22.4%; Diabetes: 9.7%; ECV: 4.1%","Minsalud Colombia 2023"],
          ["Salud mental (índice)","float32","PHQ-9 proxy: prob. depresión mayor = 0.086 (mujeres: 0.12, hombres: 0.06)","ENSM 2015"],
          ["Exposición riesgo dengue","float32","P(infección/año | altitud<1800m, urban) = 0.0039; max incidencia: 476/100k (2013)","Revisión sistemática Trop Med 2025"],
          ["Exposición riesgo malaria","float32","Endémica: Chocó, Amazonas, Vichada. Tasa mort. malaria: 0.02/100k (2018)","PMC 2021, MSPS"],
          ["Cobertura vacunación","float32","Cobertura PAI completa: 79% (urbano: 85%, rural: 68%)","INS Colombia 2023"],
          ["Afiliación salud","int8","0=contributivo(49%), 1=subsidiado(45%), 2=especial(4%), 3=vinculado(2%)","MSPS 2023"],
          ["Distancia a hospital III/IV nivel (km)","float32","Mediana urbana: 2.1km; rural: 47.3km","MSPS georreferenciación 2022"],
        ]
      ),

      h2("2.6 Perfil Psicosocial y Riesgo"),
      dataTable(
        ["Variable","Tipo","Lógica de Calibración","Fuente / Modelo"],
        [
          ["Tolerancia a ilegalidad (T_ileg)","float32 [0,1]","Función logística de estrato, desempleo prolongado, exposición a violencia en juventud","Adaptado de Fajnzylber et al. 2002"],
          ["Percepción de seguridad local","float32 [0,1]","Correlacionada con tasa de homicidios municipal (r=-0.71) y confianza institucional","Encuesta Convivencia DANE"],
          ["Victimización registrada (RUV)","bool","~9.6M personas registradas (19% población). P(registro|desplazado)=0.62","UARIV RUV 2024"],
          ["Propensión a radicalización","float32 [0,1]","Estado latente; activado si T_ileg>0.7 AND desempleo>18m AND presencia GAO en celda","Modelo endógeno"],
        ]
      ),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 3. VECTOR DE ESTADO: EMPRESA
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new PageBreak()], spacing:{before:0,after:0} }),
      h1("3. VECTOR DE ESTADO DEL AGENTE: EMPRESA / UNIDAD PRODUCTIVA"),
      dataTable(
        ["Variable","Tipo","Distribución Colombia","Fuente"],
        [
          ["Tamaño empresa","int8","Micro(92%), Pequeña(5%), Mediana(2%), Grande(1%)","DANE EMTSE 2023"],
          ["Sector CIIU","int8","Comercio/Htl(44%), Serv prof(14%), Industria(11%), Agro(8%), Construc(8%), Otro(15%)","DANE 2022"],
          ["Capital inicial (COP millones)","float32","Micro: mediana $12M, log-normal; Grande: mediana $8,400M","Supersociedades 2022"],
          ["ROA (rentabilidad sobre activos)","float32","Mediana industria: 4.2%; Comercio: 5.8%; Construcción: 3.1%","Supersociedades 2022"],
          ["Tasa de informalidad empresarial","float32","Formal: 33%; Informal (sin registro): 67%","DANE Micronegocios 2023"],
          ["Exposición a extorsión (vacuna)","float32","P(extorsión|zona conflicto, micro) ~ 0.38; costo: 8-15% ingresos brutos","FIP Colombia 2021"],
          ["Dependencia cadena vial","float32","Sectores agro y minería: alta (0.8-1.0); servicios urbanos: baja (0.1-0.3)","ANDI Encuesta logística 2022"],
          ["Adopción tecnológica","float32","Acceso banda ancha empresarial: 64% (micro: 41%, grande: 98%)","MINTIC 2023"],
        ]
      ),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 4. VECTOR DE ESTADO: GRUPOS ARMADOS
      // ═══════════════════════════════════════════════════════
      h1("4. VECTOR DE ESTADO DEL AGENTE: GRUPOS ARMADOS / CRIMEN ORGANIZADO"),
      body("Los grupos armados son agentes colectivos con vector de estado propio. Se modelan como entidades con capacidad de acción espacial, no como campos escalares estáticos. Colombia (2025) tiene operando activamente al ELN, disidencias FARC, Clan del Golfo (AGC), Los Pachenca y múltiples BACRIM regionales."),
      ...br(1),
      dataTable(
        ["Variable","Tipo","Rango / Parámetros","Fuente"],
        [
          ["Tipología","int8","0=guerrilla política, 1=GAO, 2=BACRIM, 3=cartel narcotráfico","FIP, INDEPAZ 2024"],
          ["N. combatientes","int32","ELN~5.000-6.000; Clan Golfo~8.000-14.000; Disidencias~4.000-5.000","InSight Crime 2024"],
          ["Radio influencia efectiva (km)","float32","Correlacionado con densidad de miembros por km2 y orografía","INDEPAZ informe 2024"],
          ["Economías ilícitas (USD/mes)","float32","Narcotráfico (cocaína): ~$800M-1B USD/año total; minería ilegal ~$2B/año","UNODC 2023"],
          ["Disputas territoriales activas","bool","~242 de 348 municipios frontera con uno o más grupos (UNODC 2023)","UNODC GSH 2023"],
          ["Tasa reclutamiento forzado (/año)","float32","~0.003-0.005 de población masculina 14-25 en zonas de control","Defensoría del Pueblo 2023"],
          ["Tasa homicidios selectivos","float32","Arauca 2022: 20.5/100k solo por conflicto intragrupal","IRB Canada 2023"],
          ["Prob. de cese-el-fuego","float32","Función de negociaciones activas con gobierno (campo externo)","Variable de política"],
        ]
      ),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 5. ENTORNO: GEOGRAFÍA Y ESTADO
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new PageBreak()], spacing:{before:0,after:0} }),
      h1("5. VARIABLES DEL ENTORNO: GEOGRAFÍA Y CAMPOS DE FUERZA INSTITUCIONALES"),

      h2("5.1 Capas Geoespaciales del Tablero de Juego"),
      dataTable(
        ["Capa","Resolución","Variables Contenidas","Fuente de Datos"],
        [
          ["DEM Topográfico","30m x 30m","Elevación (m), pendiente (°), aspecto, curvatura","IGAC/NASA SRTM v3"],
          ["Cobertura de suelo","250m x 250m","Bosque, páramo, pastos, cultivos, urbano, agua. Actualización anual.","IDEAM/CIAF"],
          ["Red vial","Vectorial","Km pavimentados/km2, tipo vía (4G/3G/terciaria/trocha), estado","INVIAS 2023"],
          ["Red hidrográfica","Vectorial","Ríos navegables, caudal, usos del agua, zonas de inundación","IDEAM"],
          ["Cultivos ilícitos (coca)","1km x 1km","Hectáreas por celda (hectáreas totales 2022: ~230k según UNODC)","SIMCI-UNODC 2023"],
          ["Minería ilegal","Vectorial","Polígonos de extracción, concentración mercurio (mg/L en afluentes)","ANLA/CODECHOCO 2023"],
          ["Presencia estatal","Municipio","Policías/1000 hab, efectividad judicial (índice impunidad), inversion per cápita","Justicia y Paz, DNP"],
          ["Cambio climático local","0.25° x 0.25°","Temp. media, precipitación, ENSO index, índice sequía (SPI)","IDEAM, ERA5 Reanalysis"],
        ]
      ),

      h2("5.2 Macroeconomía Nacional (Campo Externo Global)"),
      dataTable(
        ["Variable Macro","Valor Base (2024)","Impacto sobre Agentes","Fuente"],
        [
          ["TRM (COP/USD)","$4,050 COP/USD","Afecta costos importación, ingresos exportadores y remesas","Banrep ticker diario"],
          ["Tasa de intervención Banrep","9.75% EA (bajando)","Afecta costo del crédito formal y decisión de ahorro/inversión","Banrep JD 2024"],
          ["Inflación (IPC)","7.2% anual (2024)","Erosiona poder adquisitivo, activa miserias de pobreza","DANE IPC mensual"],
          ["Precio internacional del petróleo","USD 75/bbl (Brent)","Afecta regalías departamentales, empleo Llanos/Costa","CME Group"],
          ["Precio de la cocaína (farm gate)","USD ~2,600/kg (hoja procesada)","Afecta rentabilidad de economías ilícitas y reclutamiento","UNODC 2023"],
          ["PIB departamental per cápita","Bogotá: $45M COP; Chocó: $6M COP","Atractor de migración económica (modelo de gravedad)","DANE Cuentas Dptos 2022"],
        ]
      ),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 6. MÓDULO ESTOCÁSTICO: TASAS DE TRANSICIÓN CALIBRADAS
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new PageBreak()], spacing:{before:0,after:0} }),
      h1("6. MÓDULO ESTOCÁSTICO: PROBABILIDADES DE TRANSICIÓN CALIBRADAS"),
      body("Esta sección define los W(X|X') de la Ecuación Maestra. Cada probabilidad es una función condicional de las variables de estado del agente y del entorno. Las cifras provienen de fuentes primarias con datos colombianos o modelos econométricos calibrados para Colombia."),
      ...br(1),

      h2("6.1 Probabilidad de Mortalidad P(Muerte | Δt=1 año)"),
      body("Función compuesta de tres componentes independientes. Se usa la mayor de las tres como dominante, con suma probabilística para el total:"),
      formula("P(muerte_i) = 1 - [1-P_basal(edad,sexo)] · [1-P_violencia(r,grupo)] · [1-P_enfermedad(salud,acceso)]"),
      dataTable(
        ["Componente","Valor Calibrado Colombia","Condicionantes","Fuente"],
        [
          ["P_basal(edad,sexo)","Tabla de vida DANE: qx(hombre,40)=0.0031; qx(mujer,40)=0.0016","Edad, sexo biológico","DANE Tablas de vida 2020-25"],
          ["P_violencia(r)","Tasa homicidio nacional 2023: ~27/100k = 0.00027 por año","Municipio, pertenencia a GAO (x12 si combatiente), disputa territorial","UNODC 2023, Macrotrends"],
          ["P_violencia en zona de conflicto","Arauca 2022: ~398 hom/100k = 0.004/año en zona activa","Celda con disputa territorial activa","IRB Canada 2023"],
          ["P_dengue(mort | infectado)","CFR: 0.07%-0.16%; P(infección/año | altitud<1800m): 0.0039","Altitud < 1800m, cobertura vacunación baja","AJTMH 2025, revisión sistemática"],
          ["P_malaria(mort | infectado)","0.02/100k en endémicos; CFR en niños sin acceso: 2-5%","Chocó, Amazonas, Vichada + estrato bajo","PMC 2021 MSPS"],
        ]
      ),

      h2("6.2 Probabilidad de Transición Laboral (por mes)"),
      body("Modelo de matriz de Markov de 3 estados: Ocupado (O), Desocupado (D), Inactivo (I). Calibrado con GEIH mensual mediante ML (MAPE < 2.3% según arxiv:2508.12514):"),
      dataTable(
        ["Transición","Prob. Base Nacional","Moduladores Clave","Fuente"],
        [
          ["O → D (despido/renuncia)","0.018/mes (2.2%/año formal; 4.5%/año informal)","Ciclo macro: +50% si recesión; edad>55: +30%; sector construcción: +2x","DANE GEIH, Banrep"],
          ["D → O (emplearse)","0.045/mes (ciclo expansivo) / 0.028/mes (recesión)","Educación superior: +80%; estrato 1-2: -40%; municipio rural: -55%","DANE GEIH, arxiv 2508.12514"],
          ["O → I (salida mercado)","0.008/mes","Mujer con hijo <5 años: +2x; edad>60: +3x","DANE GEIH 2023"],
          ["I → O (reingreso)","0.012/mes","Choques de necesidad (deuda, muerte familiar) activan reingreso","DANE GEIH"],
          ["Formal → Informal","0.025/mes","Pérdida empleo + crisis: P=0.6 de caer a informalidad","Banrep 2023"],
          ["Informal → Formal","0.008/mes","Requiere: educación ≥ secundaria completa Y acceso crédito","Banrep 2023"],
        ]
      ),

      h2("6.3 Probabilidad de Migración Interna"),
      body("Basado en el modelo de gravedad calibrado para IDPs colombianos (Saldarriaga & Hua 2019, Cities 95:102407) y el marco UNHCR Project Jetson para desplazamiento forzado:"),
      formula("P(migración i→j) = G · [POP_j^α · EXP(-γ·d_ij)] · F_push(i) · F_pull(j) · F_red(red_social_en_j)"),
      body("Parámetros calibrados para Colombia (Registro Único de Víctimas 1986-2016):"),
      bullet("El predictor más fuerte de destino es la red de víctimas previas del mismo municipio de origen en destino j (mayor que distancia o PIB)"),
      bullet("Alpha (exponente de masa poblacional) ~ 0.85; Gamma (decaimiento por distancia) ~ 0.0023/km"),
      bullet("F_push críticos: violencia local (OR=4.2 por aumento de 10 hom/100k), desastres naturales, pérdida de cosecha"),
      bullet("F_pull críticos: PIB per cápita departamental destino, tasa de empleo, presencia previa de red familiar"),
      bullet("Centros medianos regionales (Bucaramanga, Manizales, Pereira) son atractores preferidos sobre Bogotá para IDPs rurales"),
      dataTable(
        ["Escenario de Migración","Prob. Anual Estimada","Condición","Fuente"],
        [
          ["Económica voluntaria (rural→urbana)","0.018/año en zonas sin conflicto","Diferencial PIB > 2.5x entre municipio origen y destino","DANE migraciones 2022"],
          ["Desplazamiento forzado","0.008/año en zonas de conflicto activo","Disputa territorial activa en celda de origen","UARIV RUV/IDMC 2024"],
          ["Retorno post-acuerdo de paz","0.004/año en zonas post-FARC","Firma acuerdo de paz + años de estabilidad > 2","UNHCR arxiv:2508.20662"],
          ["Migración Venezuela (entrada)","~300.000-350.000 nuevos/año (2022-2024)","Variable exógena calibrada por trimestre","Migración Colombia 2024"],
        ]
      ),

      h2("6.4 Probabilidad de Ingreso a la Ilegalidad P(Delinquir)"),
      body("Distribución logística multinomial condicionada. Función de 6 predictores con coeficientes calibrados a partir de literatura econométrica colombiana y latinoamericana:"),
      formula("P(delinquir) = σ(β₀ + β₁·desempleo_largo + β₂·estrato_bajo + β₃·exposición_violencia_joven + β₄·deserción_escolar + β₅·presencia_GAO + β₆·percepción_impunidad)"),
      dataTable(
        ["Predictor (β_k)","Coef. Aproximado","Interpretación","Fuente"],
        [
          ["Desempleo > 6 meses (β₁)","+0.85","Duplica aproximadamente el odds de delincuencia menor","Fajnzylber, Lederman, Loayza 2002"],
          ["Estrato 1 vs estrato 4+ (β₂)","+0.62","Diferencia de contexto socioeconómico","DANE-DNP"],
          ["Exposición a violencia antes de 15 años (β₃)","+0.74","Modelado como variable latente de trauma histórico","Literatura criminológica"],
          ["Deserción escolar antes de 9° grado (β₄)","+0.58","Fuerte predictor independiente","MEN Colombia"],
          ["Celda con presencia activa de GAO (β₅)","+1.12","Mayor efecto individual — disponibilidad de reclutamiento","FIP Colombia 2021"],
          ["Percepción de impunidad > 80% (β₆)","+0.49","Desincentivo a la denuncia y señal de riesgo bajo","UNODC Corruption Survey"],
        ]
      ),

      h2("6.5 Dinámica Epidémica sobre Red Social (Módulo SEIR)"),
      body("La propagación de enfermedades, ideas o innovaciones tecnológicas ocurre sobre la topología de red Barabási-Albert con gamma~2.5. Para enfermedades infecciosas se usa el modelo SEIR vectorial. Parámetros calibrados para dengue en Colombia:"),
      dataTable(
        ["Parámetro","Valor Colombia","Descripción","Fuente"],
        [
          ["Beta efectivo (tasa contacto)","0.172/día (Q1) – 0.215/día (Q4)","Varía estacionalmente con lluvia y temperatura","PubMed 31373583"],
          ["Periodo de latencia (1/sigma)","4-6 días","Tiempo exposición→infeccioso","AJTMH 2025"],
          ["Tasa de recuperación (gamma)","1/7.3 días – 1/5.9 días (estacional)","Duración promedio enfermedad","PubMed 31373583"],
          ["R0 básico (sin intervención)","1.02-2.03 según trimestre y año","Calculado por regresión no lineal sobre datos SIVIGILA","PubMed 31373583"],
          ["CFR (dengue grave sin atención)","0.89-2.7%","Varía con acceso hospitalario y coinfecciones","AJTMH 2025"],
          ["Efecto altitud","P(infección)≈0 si altitud>2200m","Factor protector biológico (Aedes aegypti)","Modelo espacial GAMM"],
        ]
      ),

      h2("6.6 Dinámica de Opinión / Radicalización (Voter Model Modificado)"),
      body("El nivel de radicalización política o propensión a apoyar grupos armados sigue un modelo de Ising/Voter adaptado a red scale-free. Un agente i en estado spin s_i ∈ {-1,+1} (pro-estado vs. pro-ilegalidad) cambia con probabilidad:"),
      formula("P(s_i → -s_i) = exp(-2·s_i·h_eff) / (1 + exp(-2·s_i·h_eff))"),
      body("donde h_eff = J·SUM_j(A_ij·s_j) + H_ext es el campo efectivo local (J=fuerza de interacción social) y H_ext es el campo externo institucional (presencia estatal, medios, narrativa gubernamental). Solo el 1% de los hubs (nodos altamente conectados) controlando su spin puede revertir el equilibrio del sistema completo."),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 7. MÓDULOS AVANZADOS
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new PageBreak()], spacing:{before:0,after:0} }),
      h1("7. MÓDULOS AVANZADOS: TEORÍA DE JUEGOS Y ECONOFÍSICA"),

      h2("7.1 Dinámica de Conflicto: Replicador con Difusión Espacial"),
      body("Los grupos armados compiten por territorio siguiendo la ecuación de dinámica del replicador con difusión espacial (sistema análogo a Lotka-Volterra modificado):"),
      formula("∂ρ_i(r,t)/∂t = D_i·∇²ρ_i(r,t) + ρ_i(r,t)·[f_i(ρ) - <f>]"),
      body("donde D_i es el coeficiente de movilidad territorial (dependiente de infraestructura vial y orografía), f_i(ρ) es el fitness local (economías ilícitas disponibles menos costo de operación), y <f> es el fitness promedio del entorno. La coca actúa como el recurso trófico: las hectáreas de coca en la celda predicen la densidad de grupos armados con una deforestación modelo logístico que fue 78% preciso a resolución de 10km² (arxiv:Davalos et al.)."),

      h2("7.2 Matriz de Pagos del Dilema de Cooperación/Deserción"),
      dataTable(
        ["Estrategia Agente","Cooperar (estado de derecho)","Desertar (delincuencia/rebelión)"],
        [
          ["Cooperar (vecino)","R = 1.0 (recompensa mutua)","S = -0.4 (sucker's payoff)"],
          ["Desertar (vecino)","T = 1.3 (tentación)","P = 0.2 (penalización mutua baja en zonas de impunidad alta)"],
        ]
      ),
      body("El parámetro P = 0.2 (en lugar de 0.0 en el dilema clásico) refleja que en municipios con impunidad > 80%, la penalización por delincuencia es sustancialmente baja. Este es el multiplicador crítico del crimen organizado en Colombia. La condición para que la cooperación sea evolutivamente estable es: R > (T+S)/2, que falla cuando P(impunidad) > 0.7."),

      h2("7.3 Econofísica: Intercambio Cinético de Capital"),
      body("Las transacciones económicas entre agentes (salarios, pagos, extorsiones, remesas) se modelan como colisiones cinéticas con conservación local de capital:"),
      formula("w_i(t+1) = w_i + ε·(w_i + w_j)·(1 - λ_i)"),
      formula("w_j(t+1) = w_j - ε·(w_i + w_j)·(1 - λ_j)"),
      body("donde epsilon es el parámetro de asimetría de la transacción (fracción que cambia de manos), lambda_i,j son las propensiones marginales al ahorro individuales. Esta dinámica reprodduce naturalmente la distribución Gibbs-Boltzmann para bajos ingresos empalmada con Pareto (alpha~1.9) para el 5% más rico — consistente con el Gini=53.9 medido en Colombia 2023."),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 8. CONDICIONES DE ITERACIÓN TEMPORAL
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new PageBreak()], spacing:{before:0,after:0} }),
      h1("8. CONDICIONES DECISIVAS DE CADA ITERACIÓN TEMPORAL"),
      body("Esta sección define el ALGORITMO DE UN PASO de simulación. Cada tick Δt (recomendado: Δt=1 semana para capturar tanto dinámicas rápidas como lentas) ejecuta los siguientes 14 pasos en orden estricto:"),
      ...br(1),

      infoBox("PASO 1", "CHOQUE MACROECONÓMICO GLOBAL. Actualizar variables externas: TRM, inflación, precio petróleo, precio cocaína. Ajustar el Hamiltoniano del sistema. Si hay choque exógeno (elecciones, pandemia, acuerdo de paz), inyectar como campo externo H_ext en el Voter Model y como shock al drift μ(w,r,t) de todas las SDE de capital."),
      ...br(1),
      infoBox("PASO 2", "ACTUALIZAR ENTORNO ESPACIAL. Recalcular capas geoespaciales dinámicas: hectáreas coca (función de precios+erradicación), cobertura forestal (modelo deforestación con 78% acc. a 10km²), precipitación IDEAM, estado vías (deterioro estacional). Recomponer tensor métrico g_uv(r,t)."),
      ...br(1),
      infoBox("PASO 3", "DINÁMICA DE GRUPOS ARMADOS. Integrar ecuación del replicador espacial Δt para cada grupo. Calcular P(expansión territorial), P(repliegue), P(reclutamiento forzado) por celda. Ejecutar acciones estratégicas (bloqueos, hostigamientos) como eventos de Poisson. Actualizar matriz de control territorial."),
      ...br(1),
      infoBox("PASO 4", "PROPAGACIÓN EPIDÉMICA. Integrar modelo SEIR sobre la red Barabási-Albert. Para cada agente en estado E (expuesto) calcular transición a I (infeccioso) según sigma y altitud. Para I, calcular recuperación o muerte según CFR y acceso hospitalario. Propagar sobre red de contactos A_ij(t)."),
      ...br(1),
      infoBox("PASO 5", "TRANSICIONES LABORALES. Para cada agente ocupado: calcular P(O→D) condicional al ciclo macro y sector. Para desocupados: calcular P(D→O) condicional a educación, municipio y oferta local. Para inactivos: activar P(I→O) si ocurre choque de necesidad (deuda crítica, muerte de proveedor). Actualizar ingresos mediante SDE de Langevin."),
      ...br(1),
      infoBox("PASO 6", "ACTUALIZACIÓN FINANCIERA. Para cada agente: integrar dw_i = μ·dt + σ·dW_i. Calcular pagos de deuda (formal e informal). Si w_i < umbral de subsistencia, activar estado de crisis financiera. Las microempresas calculan flujo de caja y P(quiebra) como función de extorsión acumulada y costo logístico. Las empresas formales pagan impuestos al campo E(r,t) de gasto público."),
      ...br(1),
      infoBox("PASO 7", "DECISIÓN DE MIGRACIÓN. Para cada agente i: calcular P(migrar) = G·F_push(i)·F_pull(j)·F_red usando modelo de gravedad calibrado. Si evento de Poisson activa migración, mover agente a nuevo municipio j vía geodésica óptima. Actualizar red social A_ij(t). Si agente cruza nodo de cómputo (MPI), empaquetar S_i y enviar via MPI_Send."),
      ...br(1),
      infoBox("PASO 8", "DECISIÓN DE INGRESO A ILEGALIDAD. Para cada agente inactivo/desocupado en zona de presencia GAO: evaluar función logística P(delinquir). Usar función de partición (Boltzmann) para elegir entre opciones: economía formal, informal, economía ilícita. Si reclutado: actualizar tipo de agente a 'combatiente'; propagar en red social del GAO."),
      ...br(1),
      infoBox("PASO 9", "ACTUALIZACIÓN DE RED SOCIAL. Agregar aristas nuevas (Barabási-Albert: nuevos nodos preferiblemente hacia hubs). Eliminar aristas por migración, muerte, encarcelamiento. Recalcular grados k_i. Propagar información, innovaciones tecnológicas y rumores sobre la red: velocidad de propagación ~ O(log N) en redes scale-free."),
      ...br(1),
      infoBox("PASO 10", "DINÁMICA DE OPINIÓN / RADICALIZACIÓN. Actualizar spin s_i para cada agente usando Voter Model modificado: h_eff = J·SUM_j(A_ij·s_j) + H_ext. El campo H_ext incluye presencia de medios, discurso político local y presencia institucional (SENA, ICBF). Si s_i < umbral: activar propensión a radicalización."),
      ...br(1),
      infoBox("PASO 11", "NACIMIENTOS Y MUERTES. Calcular P(nacimiento) para mujeres en edad fértil según estrato (TFR: estrato 1=2.8; estrato 6=1.4). Calcular P(muerte_i) compuesta. Crear/eliminar agentes. Inicializar nuevos agentes con herencia parcial del vector de estado de la madre (capital educativo, etnia, estrato) más ruido estocástico."),
      ...br(1),
      infoBox("PASO 12", "CIERRE MICRO-MACRO (ACOPLAMIENTO BIDIRECCIONAL). Agregar decisiones individuales al PIB, inflación y empleo nacional. Actualizar modelo CGE acoplado (o sistema de ecuaciones de campo medio) con las nuevas sumas. Los cambios macro realimentan los campos de fuerza externos del siguiente tick. Sin este paso, el modelo es un juguete sin coherencia macroeconómica."),
      ...br(1),
      infoBox("PASO 13", "ASIMILACIÓN DE DATOS (ENSEMBLE KALMAN FILTER). Si Δt corresponde a fin de mes: inyectar datos reales disponibles (DANE, Banrep, INS) para corregir las trayectorias del sistema. El EnKF actualiza el vector de estado global S(t) minimizando la covarianza del error: K = P_f·H^T·(H·P_f·H^T + R)^(-1), donde P_f es la covarianza del ensemble de predicción y R es la covarianza del error de observación. Sin este paso el error se acumula caóticamente."),
      ...br(1),
      infoBox("PASO 14", "LOG Y DIAGNÓSTICO COMPUTACIONAL. Calcular función de pérdida multivariada L(t) sobre 5 métricas de validación: tasa de homicidios, desempleo, inflación, hectáreas coca, tasa de desplazamiento. Si L(t) > umbral de divergencia: triggear alerta y reiniciar desde último checkpoint. Guardar snapshot del estado global cada N_save ticks. Verificar conservación de capital macro (error < 0.1%)."),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 9. ARQUITECTURA HPC
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new PageBreak()], spacing:{before:0,after:0} }),
      h1("9. ARQUITECTURA COMPUTACIONAL DE ALTO RENDIMIENTO (HPC)"),

      h2("9.1 Esquema Híbrido MPI + CUDA"),
      body("Para N=50M agentes con ~80 variables float32 cada uno: estado global ~ 16 GB. La actualización completa por tick requiere ~10^12 operaciones de punto flotante. Un solo nodo CPU es inviable. La arquitectura mínima viable para una simulación realista:"),
      dataTable(
        ["Capa","Tecnología","Función","Consideración Crítica"],
        [
          ["Descomposición de Dominio","MPI (OpenMPI o MPICH)","Dividir el territorio en regiones (Andina, Caribe, Pacífico, Orinoquía, Amazonia, Bogotá)","Ghost zones en fronteras para agentes en tránsito"],
          ["Paralelismo intra-nodo","CUDA (NVIDIA A100/H100)","Actualizar millones de agentes en paralelo dentro de cada GPU","Struct-of-Arrays (SoA) obligatorio para memoria coalescente"],
          ["Números aleatorios","cuRAND (CUDA)","Mersenne Twister paralelo — una semilla independiente por hilo","Sin esto se generan clústeres artificiales de violencia/enfermedad"],
          ["Integración SDE","Euler-Maruyama en GPU","Integrar w_i(t+dt) en paralelo para todos los agentes","Paso dt < (sigma²/2mu) para estabilidad numérica"],
          ["Paso de mensajes","MPI_Send/MPI_Recv","Transferir S_i de agentes migrantes entre nodos","Buffer de migración: ~100k agentes/mes por frontera interdepartamental"],
          ["Reducción macro","MPI_Allreduce","Agregar sumas para cierre micro-macro (PIB, empleo)","Una sola operación colectiva al final de cada tick"],
          ["Almacenamiento","HDF5 paralelo","Snapshots del estado global cada N ticks","~16 GB/snapshot — compresión GZIP-6 reduce a ~2 GB"],
        ]
      ),

      h2("9.2 Manejo de la Topología de Red Social"),
      body("La matriz de adyacencia A_ij(t) de 50M x 50M agentes es imposible en memoria densa (~20 PB). Solución: almacenamiento disperso CSR (Compressed Sparse Row) con grado medio k̄ ~ 150 aristas/agente → ~7.5B entradas → ~60 GB en float16. Usar cuSPARSE para operaciones matriciales dispersas en GPU."),
      body("Actualización de A_ij(t): solo se modifican las aristas afectadas por muertes, migraciones y nuevos nacimientos (~0.05% del grafo por tick). No recalcular el grafo completo en cada paso."),

      h2("9.3 Generación de Condiciones Iniciales con Monte Carlo"),
      body("Los microdatos colombianos (GEIH, SISBEN IV, RUV) tienen subregistro sistemático de ~25-40% en zonas rurales de conflicto. Inicializar la simulación asumiendo exactitud de los datos propagaría error caótico. Protocolo:"),
      bullet("Construir un ensemble de K=50 configuraciones iniciales, cada una perturbada con ruido gaussiano calibrado a la incertidumbre de los datos de origen"),
      bullet("Correr el ensemble en paralelo durante los primeros 12 pasos temporales"),
      bullet("Estimar la varianza inter-ensemble como estimador de la incertidumbre del estado inicial"),
      bullet("Usar la media del ensemble como estado inicial de la simulación operacional"),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 10. CALIBRACIÓN Y VALIDACIÓN
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new PageBreak()], spacing:{before:0,after:0} }),
      h1("10. CALIBRACIÓN, VALIDACIÓN Y FUNCIÓN DE PÉRDIDA"),

      h2("10.1 Función de Pérdida Multivariada"),
      body("El modelo se calibra minimizando una función de pérdida ponderada que contrasta simultáneamente 5 salidas observables contra series de tiempo reales (2010-2023):"),
      formula("L(θ) = w₁·||h_sim - h_obs||² + w₂·||u_sim - u_obs||² + w₃·||π_sim - π_obs||² + w₄·||coca_sim - coca_obs||² + w₅·||IDPs_sim - IDPs_obs||²"),
      dataTable(
        ["Objetivo de Validación (y_k)","Variable Observada","Fuente de Datos","Peso w_k Sugerido"],
        [
          ["Tasa de homicidios municipal","27/100k nacional (2021); alta varianza departamental","DANE, SIJIN, UNODC","0.25 (alta importancia política)"],
          ["Tasa de desempleo departamental","9.9% nacional (2024); MAPE<2.3% en arxiv:2508.12514","DANE GEIH mensual","0.20"],
          ["Inflación (IPC)","7.2% anual (2024), histórico 2010-2023","DANE IPC mensual","0.15"],
          ["Hectáreas de coca","230k ha (2022); tiempo serie SIMCI 2000-2023","UNODC/SIMCI anual","0.20"],
          ["IDPs registrados (nuevos/año)","~100k-300k nuevos/año según conflicto","UARIV RUV, IDMC anual","0.20"],
        ]
      ),

      h2("10.2 Técnica de Calibración Recomendada: DeepIMC + ABC"),
      body("La literatura más reciente (arxiv:2509.07013, DeepIMC) propone entrenar una red BiLSTM para aprender la función inversa: de series de tiempo observadas a parámetros del ABM. Esto elimina la necesidad de runs iterativos de ABC (que requieren 10^4-10^6 simulaciones). El flujo de trabajo:"),
      bullet("Generar 5.000 escenarios sintéticos con parámetros muestreados por hipercubo latino"),
      bullet("Entrenar red BiLSTM (DeepIMC) para mapear {series_simuladas} → {parámetros_θ}"),
      bullet("Con datos reales de Colombia, aplicar DeepIMC para obtener θ_hat en segundos"),
      bullet("Validar θ_hat con Approximate Bayesian Computation (ABC-MCMC) como ground truth"),
      bullet("Actualizar los hiperparámetros dinámicamente cuando llegan nuevos datos DANE (EnKF)"),

      h2("10.3 Reducción de Dimensionalidad del Espacio de Estado"),
      body("Con 50M agentes × 80 variables = 4×10^9 dimensiones, la inferencia bayesiana directa es imposible (maldición de la dimensionalidad). Solución obligatoria:"),
      bullet("Aplicar UMAP o t-SNE sobre una muestra representativa estratificada para encontrar la variedad de baja dimensión que captura >95% de la varianza del sistema"),
      bullet("Entrenar un Autoencoder Variacional (VAE) que comprima el estado a un espacio latente de ~500-1000 dimensiones"),
      bullet("Realizar la inferencia bayesiana en el espacio latente; decodificar para diagnóstico"),
      bullet("El espacio latente también permite detectar 'fases' del sistema: transiciones abruptas (paz→guerra, bonanza→recesión) como discontinuidades en la variedad latente"),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 11. BRECHAS CRÍTICAS
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new PageBreak()], spacing:{before:0,after:0} }),
      h1("11. BRECHAS CRÍTICAS Y PROBLEMAS ABIERTOS"),
      body("Las siguientes son las debilidades estructurales que, si no se resuelven, invalidan la utilidad predictiva del modelo independientemente de cuán detallado sea su parametrizado:"),
      ...br(1),

      infoBox("BRECHA 1: HAMILTONIANO AUSENTE", "No se ha definido la 'energía' total del sistema socioeconómico. Sin una función de Lyapunov o análogo del Hamiltoniano H(S,E) que determine qué estados son más probables, no existe manera analítica de saber si la simulación converge hacia un atractor, un estado estacionario, o diverge numéricamente. SOLUCIÓN: definir H como la suma ponderada de potenciales individuales (deuda, vulnerabilidad, violencia) y verificar que dH/dt ≤ 0 promediado sobre el ensemble."),
      ...br(1),
      infoBox("BRECHA 2: CIERRE MICRO-MACRO", "Las reglas de los agentes reaccionan a la economía pero falta el mecanismo por el cual la suma de millones de decisiones altera la inflación y el PIB en t+1. SOLUCIÓN MÍNIMA: acoplar a un modelo de ecuaciones de campo medio calibrado (ej. sistema de ODEs logísticas que mapeen empleo_macro, informalidad_macro, gasto_macro a partir de agregados del ABM)."),
      ...br(1),
      infoBox("BRECHA 3: ANCHO DE BANDA DE MEMORIA", "Con 80 variables float32 por agente, el estado global ocupa ~16 GB. La actualización de la red social dispersa (A_ij) requiere accesos no coalescentes que saturan el bus PCI-e. SOLUCIÓN: implementar Struct-of-Arrays (SoA) en lugar de Array-of-Structs (AoS). Agrupar variables de acceso frecuente por pasos (ej. todas las coordenadas juntas, todos los ingresos juntos) para garantizar acceso coalescente en GPU."),
      ...br(1),
      infoBox("BRECHA 4: DISCRETIZACIÓN TEMPORAL MULTIESCALA", "La información en redes sociales se propaga en horas; las variables demográficas (envejecimiento, escolaridad) cambian en años. Un Δt único crea aliasing. SOLUCIÓN: usar simulación de eventos discretos asincrónica (algoritmo de Gillespie adaptado) para los procesos rápidos, y actualización síncrona con Δt=1 semana para los procesos lentos. El scheduler debe gestionar dos colas de eventos con prioridades diferentes."),
      ...br(1),
      infoBox("BRECHA 5: VALIDACIÓN DE EMERGENCIA MACRO", "Si el modelo no reproduce naturalmente las leyes empíricas macroscópicas (distribución Pareto de ingreso, tasa de urbanización histórica, ciclos de violencia) a partir de reglas microscópicas SIN FORZARLAS, la física estadística del modelo está mal formulada. CRITERIO DE ACEPTACIÓN: el modelo pasa validación si reproduce sin calibración forzada el Gini~54, la informalidad~56% y la distribución espacial de coca (R²>0.7 con SIMCI) como propiedades emergentes."),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 12. HOJA DE RUTA DE IMPLEMENTACIÓN
      // ═══════════════════════════════════════════════════════
      h1("12. HOJA DE RUTA DE IMPLEMENTACIÓN"),
      dataTable(
        ["Fase","Descripción","Entregable","Duración Estimada"],
        [
          ["FASE 0","Infraestructura de datos: descargar y procesar DEM IGAC, GEIH 2023, RUV, SIMCI, SIVIGILA. Construir base de datos geoespacial municipal.","Base de datos PostgreSQL/PostGIS + HDF5 de covariables.","4-6 semanas"],
          ["FASE 1","Prototipo 2D simplificado: N=100k agentes en una sola región (ej. Valle del Cauca). Implementar módulos 1-5 (mortalidad, empleo, migración, salud, delincuencia).","Simulador funcional a escala reducida. Validación vs. datos Valle del Cauca 2015-2020.","8-12 semanas"],
          ["FASE 2","Escalado nacional: paralelización MPI+CUDA. Implementar ghost zones y migración inter-nodo. Añadir módulos de grupos armados (replicador espacial) y econofísica.","Simulador N=5M a escala nacional con Δt=1 semana.","12-16 semanas"],
          ["FASE 3","Calibración automática: entrenar BiLSTM-DeepIMC sobre 5.000 escenarios sintéticos. Implementar EnKF para asimilación mensual de datos DANE.","Error de calibración L(θ) < 15% en las 5 métricas de validación.","8-10 semanas"],
          ["FASE 4","Escala completa N=50M + análisis de sensibilidad: mapa de bifurcaciones del sistema bajo diferentes políticas (paz total, reforma laboral, erradicación de coca, inversión rural).","Gemelo digital operacional. Dashboards de escenarios contrafactuales.","Continuo"],
        ]
      ),
      ...br(1),

      // ═══════════════════════════════════════════════════════
      // 13. REFERENCIAS
      // ═══════════════════════════════════════════════════════
      new Paragraph({ children:[new PageBreak()], spacing:{before:0,after:0} }),
      h1("13. REFERENCIAS PRIMARIAS Y FUENTES DE DATOS"),
      h2("13.1 Datos Colombianos"),
      bullet("DANE — Gran Encuesta Integrada de Hogares (GEIH) 2023. Microdatos laborales mensuales por departamento."),
      bullet("DANE — Censo Nacional de Población y Vivienda (CNPV) 2018. Granularidad municipal."),
      bullet("DANE — Tablas de Vida de la Población Colombiana 2020-2025. Curvas qx por edad y sexo."),
      bullet("UARIV — Registro Único de Víctimas (RUV). 9.6M personas registradas. Flujos de desplazamiento 1986-2024."),
      bullet("UNODC/SIMCI — Monitoreo de territorios afectados por cultivos ilícitos. Serie 2000-2023."),
      bullet("INS Colombia — SIVIGILA. Casos semanales de enfermedades de notificación obligatoria por municipio."),
      bullet("IGAC — DEM Nacional resolución 30m. Tensor métrico geográfico."),
      bullet("Banco de la República — Series macroeconómicas: TRM, tasas, inflación, agregados monetarios."),
      bullet("INDEPAZ / FIP — Presencia y acciones de grupos armados por municipio 2020-2024."),
      h2("13.2 Literatura Científica Clave"),
      bullet("Saldarriaga & Hua (2019). 'A gravity model analysis of forced displacement in Colombia.' Cities 95:102407. [Modelo de migración forzada]"),
      bullet("Villar et al. (2015); AJTMH (2025). 'Epidemiological trends of dengue in Colombia.' [Parámetros SEIR]"),
      bullet("Davalos et al. (2011). 'Forests and Drugs: Coca-driven deforestation.' Env. Sci. & Tech. [Modelo deforestación 78% acc.]"),
      bullet("Fajnzylber, Lederman, Loayza (2002). 'Inequality and Violent Crime.' J. Law & Economics. [Coeficientes logísticos crimen]"),
      bullet("Saldarriaga et al. (arxiv:2508.12514, 2025). 'Reconstructing Subnational Labor Indicators in Colombia.' [MAPE<2.3% modelo laboral]"),
      bullet("Barabási & Albert (1999). 'Emergence of scaling in random networks.' Science. [Topología red social]"),
      bullet("Gardiner, C.W. 'Handbook of Stochastic Methods.' Springer. [Formalismo SDE, Fokker-Planck]"),
      bullet("arxiv:2509.07013 (2026). 'DeepIMC: Generalized ML for Fast Calibration of Agent-Based Epidemic Models.' [Calibración BiLSTM]"),
      bullet("arxiv:2508.20662 (2025). 'The Aftermath of Peace: FARC ceasefire and forced displacement in Colombia.' [EnKF desplazamiento]"),
      bullet("UNODC Global Study on Homicide 2023. [242/348 municipios fronterizos con grupos armados]"),
      bullet("FLAME GPU 2. Documentación técnica. flamegpu.com. [Framework HPC para ABM masivos en GPU]"),
      ...br(2),

      new Paragraph({
        children:[new TextRun({text:"FIN DEL DOCUMENTO MAESTRO", font:"Arial", size:24, bold:true, color:C.gold})],
        alignment:AlignmentType.CENTER,
        spacing:{before:400,after:0},
        border:{top:{style:BorderStyle.THICK, size:6, color:C.gold, space:8}}
      }),
      new Paragraph({
        children:[new TextRun({text:"Versión 1.0 — Junio 2026 | Clasificación: Confidencial — Sebastián", font:"Arial", size:18, italics:true, color:"888888"})],
        alignment:AlignmentType.CENTER,
        spacing:{before:60,after:0}
      }),
    ]
  }]
});

const OUT = process.env.SOCIUM_DOCX_OUT || "docs/spec/ABM_Colombia_Especificacion_Maestra.docx";
Packer.toBuffer(doc).then(buf => {
  fs.writeFileSync(OUT, buf);
  console.log('OK — documento generado en ' + OUT);
});
